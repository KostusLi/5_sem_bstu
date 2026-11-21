CREATE OR REPLACE PACKAGE PKG_DATA_GEN AS
    -- Полная генерация тестовых данных клиники (374 400 слотов расписания за 4 года)
    PROCEDURE generate_all_data;
END PKG_DATA_GEN;


CREATE OR REPLACE PACKAGE BODY PKG_DATA_GEN AS

    PROCEDURE generate_all_data IS
        v_cnt          NUMBER;
        v_admin_role   NUMBER;
        v_doc_role     NUMBER;
        v_reg_role     NUMBER;
        v_pass_admin   VARCHAR2(128) := PKG_COMMON.fn_hash_password('admin123');
        v_pass_doc     VARCHAR2(128) := PKG_COMMON.fn_hash_password('doc123');
        v_pass_reg     VARCHAR2(128) := PKG_COMMON.fn_hash_password('reg123');
        v_user_id      NUMBER;
        v_spec_id      NUMBER;
    BEGIN
        -- Проверяем, не заполнена ли уже база
        SELECT COUNT(*) INTO v_cnt FROM ROLES;
        IF v_cnt > 0 THEN
            RAISE_APPLICATION_ERROR(-20090, 'База данных уже содержит данные. Повторная генерация отменена.');
        END IF;

        -- 1. Заполняем роли (ROLES)
        INSERT INTO ROLES (role_name) VALUES ('ADMIN')     RETURNING role_id INTO v_admin_role;
        INSERT INTO ROLES (role_name) VALUES ('DOCTOR')    RETURNING role_id INTO v_doc_role;
        INSERT INTO ROLES (role_name) VALUES ('REGISTRAR') RETURNING role_id INTO v_reg_role;

        -- 2. Заполняем 7 медицинских специальностей (SPECIALTIES)
        INSERT INTO SPECIALTIES (specialty_name) VALUES ('Терапия');
        INSERT INTO SPECIALTIES (specialty_name) VALUES ('Кардиология');
        INSERT INTO SPECIALTIES (specialty_name) VALUES ('Неврология');
        INSERT INTO SPECIALTIES (specialty_name) VALUES ('Офтальмология');
        INSERT INTO SPECIALTIES (specialty_name) VALUES ('Хирургия');
        INSERT INTO SPECIALTIES (specialty_name) VALUES ('Эндокринология');
        INSERT INTO SPECIALTIES (specialty_name) VALUES ('Отоларингология');

        -- 3. Заполняем прайс-лист услуг (SERVICES) - по 4 услуги на каждую специальность (28 услуг)
        FOR r_sp IN (SELECT specialty_id, specialty_name, ROWNUM AS rn FROM SPECIALTIES ORDER BY specialty_id) LOOP
            INSERT INTO SERVICES (specialty_id, service_name, price)
            VALUES (r_sp.specialty_id, 'Первичный приём (' || r_sp.specialty_name || ')', 45.00 + r_sp.rn * 5);

            INSERT INTO SERVICES (specialty_id, service_name, price)
            VALUES (r_sp.specialty_id, 'Повторная консультация (' || r_sp.specialty_name || ')', 30.00 + r_sp.rn * 3);

            INSERT INTO SERVICES (specialty_id, service_name, price)
            VALUES (r_sp.specialty_id, 'Комплексная диагностика (' || r_sp.specialty_name || ')', 85.00 + r_sp.rn * 10);

            INSERT INTO SERVICES (specialty_id, service_name, price)
            VALUES (r_sp.specialty_id, 'Лечебная процедура / манипуляция (' || r_sp.specialty_name || ')', 60.00 + r_sp.rn * 7);
        END LOOP;

        -- 4. Создаём учётные записи администратора и 2 регистраторов
        INSERT INTO USERS (role_id, login, password_hash, full_name, phone, is_active)
        VALUES (v_admin_role, 'admin', v_pass_admin, 'Хаткевич Константин Владимирович', '+375291000001', 1);

        INSERT INTO USERS (role_id, login, password_hash, full_name, phone, is_active)
        VALUES (v_reg_role, 'registrar1', v_pass_reg, 'Ковалёва Анна Сергеевна', '+375291000002', 1);

        INSERT INTO USERS (role_id, login, password_hash, full_name, phone, is_active)
        VALUES (v_reg_role, 'registrar2', v_pass_reg, 'Савицкая Мария Игоревна', '+375291000003', 1);

        -- 5. Создаём ровно 20 врачей (D = 20), распределённых по 7 специальностям
        FOR i IN 1..20 LOOP
            SELECT specialty_id INTO v_spec_id
            FROM (
                SELECT specialty_id, ROW_NUMBER() OVER (ORDER BY specialty_id) AS rn
                FROM SPECIALTIES
            )
            WHERE rn = MOD(i - 1, 7) + 1;

            INSERT INTO USERS (role_id, login, password_hash, full_name, phone, is_active)
            VALUES (
                v_doc_role,
                'doctor' || i,
                v_pass_doc,
                CASE MOD(i, 10)
                    WHEN 1 THEN 'Иванов Алексей Петрович (' || i || ')'
                    WHEN 2 THEN 'Смирнова Елена Викторовна (' || i || ')'
                    WHEN 3 THEN 'Кузнецов Дмитрий Сергеевич (' || i || ')'
                    WHEN 4 THEN 'Попова Ольга Андреевна (' || i || ')'
                    WHEN 5 THEN 'Васильев Павел Николаевич (' || i || ')'
                    WHEN 6 THEN 'Новикова Татьяна Михайловна (' || i || ')'
                    WHEN 7 THEN 'Морозов Артём Игоревич (' || i || ')'
                    WHEN 8 THEN 'Волкова Наталья Александровна (' || i || ')'
                    WHEN 9 THEN 'Соколов Максим Владимирович (' || i || ')'
                    ELSE        'Лебедева Екатерина Олеговна (' || i || ')'
                END,
                '+37529300' || LPAD(i, 4, '0'),
                1
            ) RETURNING user_id INTO v_user_id;

            INSERT INTO DOCTORS (user_id, specialty_id)
            VALUES (v_user_id, v_spec_id);
        END LOOP;

        -- 6. Создаём 3 000 карточек пациентов (PATIENTS)
        INSERT INTO PATIENTS (full_name, birth_date, phone, policy_number, registered_at)
        SELECT 
            'Пациент Клиники №' || LEVEL,
            DATE '1955-01-01' + MOD(LEVEL * 137, 18000),
            '+37544' || LPAD(1000000 + LEVEL, 7, '0'),
            'BY-MED-' || LPAD(LEVEL, 6, '0'),
            DATE '2022-01-01' + MOD(LEVEL, 1000)
        FROM DUAL
        CONNECT BY LEVEL <= 3000;

        COMMIT;

        -- 7. Генерируем ровно 374 400 слотов расписания (20 врачей * 18 слотов * 1040 рабочих дней)
        -- Период охватывает 4 года (2023-2026)
        INSERT /*+ APPEND */ INTO APPOINTMENTS (doctor_id, patient_id, slot_start, cabinet_number, status)
        WITH work_days AS (
            SELECT work_date, ROWNUM AS day_num
            FROM (
                SELECT DATE '2023-01-02' + (LEVEL - 1) AS work_date
                FROM DUAL
                CONNECT BY LEVEL <= 1500
            )
            WHERE TRUNC(work_date) - TRUNC(work_date, 'IW') < 5 -- Только Пн-Пт
            FETCH FIRST 1040 ROWS ONLY
        ),
        time_slots AS (
            -- 18 слотов в день с 09:00 до 17:30 по 30 минут
            SELECT 
                LEVEL AS slot_num,
                NUMTODSINTERVAL(9 * 60 + (LEVEL - 1) * 30, 'MINUTE') AS slot_offset
            FROM DUAL
            CONNECT BY LEVEL <= 18
        ),
        doctors_list AS (
            SELECT doctor_id, ROWNUM AS doc_num
            FROM DOCTORS
            ORDER BY doctor_id
        ),
        raw_grid AS (
            SELECT 
                d.doctor_id,
                d.doc_num,
                w.day_num,
                t.slot_num,
                CAST(w.work_date AS TIMESTAMP(0)) + t.slot_offset AS slot_start,
                100 + d.doc_num AS cabinet_number,
                MOD(d.doc_num * 31 + w.day_num * 17 + t.slot_num * 13, 100) AS rand_val
            FROM doctors_list d
            CROSS JOIN work_days w
            CROSS JOIN time_slots t
        )
        SELECT 
            doctor_id,
            CASE 
                WHEN rand_val < 65 THEN MOD(doc_num * 97 + day_num * 23 + slot_num * 7, 3000) + 1
                ELSE NULL
            END AS patient_id,
            slot_start,
            cabinet_number,
            CASE 
                WHEN rand_val < 55 THEN 'COMPLETED'
                WHEN rand_val < 65 THEN 'BOOKED'
                WHEN rand_val < 95 THEN 'AVAILABLE'
                ELSE 'CANCELLED'
            END AS status
        FROM raw_grid;

        COMMIT;

        -- 8. Заполняем оказанные услуги (APPOINTMENT_SERVICES) для всех завершённых приёмов (COMPLETED)
        INSERT /*+ APPEND */ INTO APPOINTMENT_SERVICES (appointment_id, service_id, quantity, current_price)
        WITH doc_primary_service AS (
            SELECT 
                d.doctor_id,
                MIN(s.service_id) AS first_service_id,
                MAX(s.service_id) AS last_service_id,
                MIN(s.price)      AS first_price,
                MAX(s.price)      AS last_price
            FROM DOCTORS d
            JOIN SERVICES s ON d.specialty_id = s.specialty_id
            GROUP BY d.doctor_id
        )
        SELECT 
            a.appointment_id,
            CASE WHEN MOD(a.appointment_id, 3) = 0 THEN dps.last_service_id ELSE dps.first_service_id END,
            CASE WHEN MOD(a.appointment_id, 5) = 0 THEN 2 ELSE 1 END,
            CASE WHEN MOD(a.appointment_id, 3) = 0 THEN dps.last_price ELSE dps.first_price END
        FROM APPOINTMENTS a
        JOIN doc_primary_service dps ON a.doctor_id = dps.doctor_id
        WHERE a.status = 'COMPLETED';

        COMMIT;

        -- 9. Заполняем счета и платежи (PAYMENTS) для завершённых приёмов
        INSERT /*+ APPEND */ INTO PAYMENTS (appointment_id, amount, discount_percent, payment_date, status)
        SELECT 
            aps.appointment_id,
            SUM(aps.current_price * aps.quantity) AS amount,
            CASE 
                WHEN MOD(aps.appointment_id, 10) = 0 THEN 10
                WHEN MOD(aps.appointment_id, 7) = 0  THEN 5
                ELSE 0
            END AS discount_percent,
            CASE 
                WHEN MOD(aps.appointment_id, 12) = 0 THEN NULL
                ELSE TRUNC( CAST(a.slot_start AS DATE) )
            END AS payment_date,
            CASE 
                WHEN MOD(aps.appointment_id, 12) = 0 THEN 'UNPAID'
                ELSE 'PAID'
            END AS status
        FROM APPOINTMENT_SERVICES aps
        JOIN APPOINTMENTS a ON aps.appointment_id = a.appointment_id
        GROUP BY aps.appointment_id, a.slot_start;

        COMMIT;

        -- 10. Заполняем электронные медицинские карты (MEDICAL_RECORDS) с документами JSON
        INSERT INTO MEDICAL_RECORDS (appointment_id, complaints, diagnosis, prescriptions, analyses_results)
        SELECT 
            appointment_id,
            'Жалобы на повышенную утомляемость, периодические боли и дискомфорт (приём №' || appointment_id || ')',
            CASE MOD(appointment_id, 5)
                WHEN 0 THEN 'J06.9 Острая инфекция верхних дыхательных путей'
                WHEN 1 THEN 'I10 Эссенциальная (первичная) гипертензия'
                WHEN 2 THEN 'M54.5 Боль внизу спины (люмбалгия)'
                WHEN 3 THEN 'H52.1 Миопия средней степени'
                ELSE        'E04.1 Нетоксический одноузловой зоб'
            END,
            'Рекомендован контроль показателей через 14 дней, медикаментозная терапия по схеме.',
            JSON(
                '{"test_type": "' || CASE WHEN MOD(appointment_id, 2) = 0 THEN 'Общий анализ крови' ELSE 'Биохимия крови' END || '", ' ||
                '"hemoglobin_g_l": ' || (120 + MOD(appointment_id, 45)) || ', ' ||
                '"leukocytes_10_9_l": ' || TO_CHAR(4.5 + MOD(appointment_id, 50) / 10.0, 'FM990.0', 'NLS_NUMERIC_CHARACTERS = ''.,''') || ', ' ||
                '"glucose_mmol_l": ' || TO_CHAR(4.1 + MOD(appointment_id, 25) / 10.0, 'FM990.0', 'NLS_NUMERIC_CHARACTERS = ''.,''') || ', ' ||
                '"laboratory": "Клинико-диагностическая лаборатория №1"}'
            )
        FROM (
            SELECT appointment_id
            FROM APPOINTMENTS
            WHERE status = 'COMPLETED'
            ORDER BY appointment_id DESC
            FETCH FIRST 5000 ROWS ONLY
        );

        COMMIT;
        DBMS_STATS.GATHER_SCHEMA_STATS(ownname => USER);
    END generate_all_data;

END PKG_DATA_GEN;