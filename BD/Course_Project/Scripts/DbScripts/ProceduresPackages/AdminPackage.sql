CREATE OR REPLACE PACKAGE PKG_ADMIN AS
    -- 1.1 Добавление учётной записи сотрудника (врача или регистратора)
    PROCEDURE add_staff_user(
        p_role_name    IN  VARCHAR2,
        p_login        IN  VARCHAR2,
        p_password     IN  VARCHAR2,
        p_full_name    IN  VARCHAR2,
        p_phone        IN  VARCHAR2,
        p_specialty_id IN  NUMBER DEFAULT NULL,
        p_new_user_id  OUT NUMBER
    );

    -- 1.2 Изменение учётной записи сотрудника
    PROCEDURE update_staff_user(
        p_user_id      IN NUMBER,
        p_full_name    IN VARCHAR2,
        p_phone        IN VARCHAR2,
        p_specialty_id IN NUMBER DEFAULT NULL,
        p_is_active    IN NUMBER DEFAULT 1
    );

    -- 1.3 Удаление учётной записи (блокировка без потери истории приёмов)
    PROCEDURE deactivate_staff_user(p_user_id IN NUMBER);

    -- Просмотр списка всех сотрудников и специальностей
    PROCEDURE get_staff_list(p_cursor OUT SYS_REFCURSOR);
    PROCEDURE get_specialties(p_cursor OUT SYS_REFCURSOR);

    -- 2.1 Добавление новой медицинской услуги
    PROCEDURE add_service(
        p_specialty_id IN  NUMBER,
        p_service_name IN  VARCHAR2,
        p_price        IN  NUMBER,
        p_service_id   OUT NUMBER
    );

    -- 2.2 Изменение услуги прайс-листа
    PROCEDURE update_service(
        p_service_id   IN NUMBER,
        p_service_name IN VARCHAR2,
        p_price        IN NUMBER
    );

    -- 2.3 Удаление услуги (если она не использовалась в приёмах)
    PROCEDURE delete_service(p_service_id IN NUMBER);

    -- 3. АНАЛИТИКА 1: Финансовый отчёт о выручке по специальностям и месяцам
    PROCEDURE report_financial_by_specialty(
        p_date_from IN  DATE,
        p_date_to   IN  DATE,
        p_cursor    OUT SYS_REFCURSOR
    );

    -- 4. АНАЛИТИКА 2: Анализ загруженности врачей клиники
    PROCEDURE report_doctor_workload(
        p_date_from IN  DATE,
        p_date_to   IN  DATE,
        p_cursor    OUT SYS_REFCURSOR
    );

    -- 5. АНАЛИТИКА 3: Прогнозирование популярности услуг (тренды год к году)
    PROCEDURE report_service_trends(
        p_year_prev IN  NUMBER,
        p_year_curr IN  NUMBER,
        p_cursor    OUT SYS_REFCURSOR
    );
END PKG_ADMIN;



CREATE OR REPLACE PACKAGE BODY PKG_ADMIN AS

    PROCEDURE add_staff_user(
        p_role_name    IN  VARCHAR2,
        p_login        IN  VARCHAR2,
        p_password     IN  VARCHAR2,
        p_full_name    IN  VARCHAR2,
        p_phone        IN  VARCHAR2,
        p_specialty_id IN  NUMBER DEFAULT NULL,
        p_new_user_id  OUT NUMBER
    ) IS
        v_role_id ROLES.role_id%TYPE;
    BEGIN
        SELECT role_id INTO v_role_id
        FROM ROLES
        WHERE role_name = UPPER(TRIM(p_role_name));

        IF UPPER(TRIM(p_role_name)) = 'DOCTOR' AND p_specialty_id IS NULL THEN
            RAISE_APPLICATION_ERROR(-20020, 'Для роли DOCTOR необходимо указать специальность (specialty_id).');
        END IF;

        INSERT INTO USERS (role_id, login, password_hash, full_name, phone, is_active)
        VALUES (v_role_id, TRIM(p_login), PKG_COMMON.fn_hash_password(p_password), TRIM(p_full_name), p_phone, 1)
        RETURNING user_id INTO p_new_user_id;

        IF UPPER(TRIM(p_role_name)) = 'DOCTOR' THEN
            INSERT INTO DOCTORS (user_id, specialty_id)
            VALUES (p_new_user_id, p_specialty_id);
        END IF;

        COMMIT;
    EXCEPTION
        WHEN NO_DATA_FOUND THEN
            RAISE_APPLICATION_ERROR(-20021, 'Указанная роль не существует.');
        WHEN DUP_VAL_ON_INDEX THEN
            ROLLBACK;
            RAISE_APPLICATION_ERROR(-20022, 'Пользователь с таким логином уже существует.');
    END add_staff_user;

    PROCEDURE update_staff_user(
        p_user_id      IN NUMBER,
        p_full_name    IN VARCHAR2,
        p_phone        IN VARCHAR2,
        p_specialty_id IN NUMBER DEFAULT NULL,
        p_is_active    IN NUMBER DEFAULT 1
    ) IS
    BEGIN
        UPDATE USERS
        SET full_name = TRIM(p_full_name),
            phone     = p_phone,
            is_active = p_is_active
        WHERE user_id = p_user_id;

        IF SQL%ROWCOUNT = 0 THEN
            RAISE_APPLICATION_ERROR(-20023, 'Пользователь не найден.');
        END IF;

        IF p_specialty_id IS NOT NULL THEN
            UPDATE DOCTORS
            SET specialty_id = p_specialty_id
            WHERE user_id = p_user_id;
        END IF;

        COMMIT;
    END update_staff_user;

    PROCEDURE deactivate_staff_user(p_user_id IN NUMBER) IS
    BEGIN
        UPDATE USERS
        SET is_active = 0
        WHERE user_id = p_user_id;

        IF SQL%ROWCOUNT = 0 THEN
            RAISE_APPLICATION_ERROR(-20023, 'Пользователь не найден.');
        END IF;
        COMMIT;
    END deactivate_staff_user;

    PROCEDURE get_staff_list(p_cursor OUT SYS_REFCURSOR) IS
    BEGIN
        OPEN p_cursor FOR
            SELECT u.user_id, r.role_name, u.login, u.full_name, u.phone, u.is_active,
                   d.doctor_id, s.specialty_id, s.specialty_name
            FROM USERS u
            JOIN ROLES r ON u.role_id = r.role_id
            LEFT JOIN DOCTORS d ON u.user_id = d.user_id
            LEFT JOIN SPECIALTIES s ON d.specialty_id = s.specialty_id
            ORDER BY u.user_id;
    END get_staff_list;

    PROCEDURE get_specialties(p_cursor OUT SYS_REFCURSOR) IS
    BEGIN
        OPEN p_cursor FOR
            SELECT specialty_id, specialty_name
            FROM SPECIALTIES
            ORDER BY specialty_name;
    END get_specialties;

    PROCEDURE add_service(
        p_specialty_id IN  NUMBER,
        p_service_name IN  VARCHAR2,
        p_price        IN  NUMBER,
        p_service_id   OUT NUMBER
    ) IS
    BEGIN
        INSERT INTO SERVICES (specialty_id, service_name, price)
        VALUES (p_specialty_id, TRIM(p_service_name), p_price)
        RETURNING service_id INTO p_service_id;
        COMMIT;
    EXCEPTION
        WHEN DUP_VAL_ON_INDEX THEN
            RAISE_APPLICATION_ERROR(-20024, 'Услуга с таким названием уже существует в данной специальности.');
    END add_service;

    PROCEDURE update_service(
        p_service_id   IN NUMBER,
        p_service_name IN VARCHAR2,
        p_price        IN NUMBER
    ) IS
    BEGIN
        UPDATE SERVICES
        SET service_name = TRIM(p_service_name),
            price        = p_price
        WHERE service_id = p_service_id;

        IF SQL%ROWCOUNT = 0 THEN
            RAISE_APPLICATION_ERROR(-20025, 'Услуга не найдена.');
        END IF;
        COMMIT;
    END update_service;

    PROCEDURE delete_service(p_service_id IN NUMBER) IS
        v_used_count NUMBER;
    BEGIN
        SELECT COUNT(*) INTO v_used_count
        FROM APPOINTMENT_SERVICES
        WHERE service_id = p_service_id;

        IF v_used_count > 0 THEN
            RAISE_APPLICATION_ERROR(-20026, 'Невозможно удалить услугу, так как она уже зафиксирована в проведённых приёмах.');
        END IF;

        DELETE FROM SERVICES WHERE service_id = p_service_id;
        COMMIT;
    END delete_service;

    PROCEDURE report_financial_by_specialty(
        p_date_from IN  DATE,
        p_date_to   IN  DATE,
        p_cursor    OUT SYS_REFCURSOR
    ) IS
    BEGIN
        OPEN p_cursor FOR
            SELECT 
                sp.specialty_name,
                TO_CHAR(a.slot_start, 'YYYY-MM') AS report_month,
                COUNT(pay.payment_id)            AS paid_invoices_count,
                SUM(pay.amount)                  AS gross_amount,
                SUM(pay.final_amount)            AS net_revenue,
                ROUND(AVG(pay.final_amount), 2)  AS avg_check
            FROM PAYMENTS pay
            JOIN APPOINTMENTS a ON pay.appointment_id = a.appointment_id
            JOIN DOCTORS d      ON a.doctor_id = d.doctor_id
            JOIN SPECIALTIES sp ON d.specialty_id = sp.specialty_id
            WHERE pay.status = 'PAID'
              AND a.slot_start >= CAST(TRUNC(p_date_from) AS TIMESTAMP)
              AND a.slot_start <  CAST(TRUNC(p_date_to) + 1 AS TIMESTAMP)
            GROUP BY sp.specialty_name, TO_CHAR(a.slot_start, 'YYYY-MM')
            ORDER BY report_month DESC, net_revenue DESC;
    END report_financial_by_specialty;

    PROCEDURE report_doctor_workload(
        p_date_from IN  DATE,
        p_date_to   IN  DATE,
        p_cursor    OUT SYS_REFCURSOR
    ) IS
    BEGIN
        OPEN p_cursor FOR
            SELECT 
                d.doctor_id,
                u.full_name AS doctor_name,
                sp.specialty_name,
                COUNT(a.appointment_id) AS total_slots,
                SUM(CASE WHEN a.status IN ('BOOKED', 'COMPLETED') THEN 1 ELSE 0 END) AS busy_slots,
                SUM(CASE WHEN a.status = 'COMPLETED' THEN 1 ELSE 0 END)              AS completed_slots,
                SUM(CASE WHEN a.status = 'AVAILABLE' THEN 1 ELSE 0 END)              AS free_slots,
                ROUND(
                    SUM(CASE WHEN a.status IN ('BOOKED', 'COMPLETED') THEN 1 ELSE 0 END) * 100.0 /
                    NULLIF(COUNT(a.appointment_id), 0), 2
                ) AS workload_percent
            FROM DOCTORS d
            JOIN USERS u        ON d.user_id = u.user_id
            JOIN SPECIALTIES sp ON d.specialty_id = sp.specialty_id
            JOIN APPOINTMENTS a ON d.doctor_id = a.doctor_id
            WHERE a.slot_start >= CAST(TRUNC(p_date_from) AS TIMESTAMP)
              AND a.slot_start <  CAST(TRUNC(p_date_to) + 1 AS TIMESTAMP)
            GROUP BY d.doctor_id, u.full_name, sp.specialty_name
            ORDER BY workload_percent DESC;
    END report_doctor_workload;

    PROCEDURE report_service_trends(
        p_year_prev IN  NUMBER,
        p_year_curr IN  NUMBER,
        p_cursor    OUT SYS_REFCURSOR
    ) IS
    BEGIN
        OPEN p_cursor FOR
            WITH yearly_stats AS (
                SELECT 
                    s.service_id,
                    s.service_name,
                    sp.specialty_name,
                    SUM(CASE WHEN EXTRACT(YEAR FROM a.slot_start) = p_year_prev THEN aps.quantity ELSE 0 END) AS qty_prev_year,
                    SUM(CASE WHEN EXTRACT(YEAR FROM a.slot_start) = p_year_curr THEN aps.quantity ELSE 0 END) AS qty_curr_year,
                    SUM(CASE WHEN EXTRACT(YEAR FROM a.slot_start) = p_year_curr THEN aps.quantity * aps.current_price ELSE 0 END) AS revenue_curr_year
                FROM SERVICES s
                JOIN SPECIALTIES sp           ON s.specialty_id = sp.specialty_id
                LEFT JOIN APPOINTMENT_SERVICES aps ON s.service_id = aps.service_id
                LEFT JOIN APPOINTMENTS a      ON aps.appointment_id = a.appointment_id
                                             AND a.status = 'COMPLETED'
                                             AND EXTRACT(YEAR FROM a.slot_start) IN (p_year_prev, p_year_curr)
                GROUP BY s.service_id, s.service_name, sp.specialty_name
            )
            SELECT 
                service_id,
                specialty_name,
                service_name,
                qty_prev_year,
                qty_curr_year,
                (qty_curr_year - qty_prev_year) AS absolute_growth,
                CASE 
                    WHEN qty_prev_year = 0 AND qty_curr_year > 0 THEN 100.0
                    WHEN qty_prev_year = 0 THEN 0.0
                    ELSE ROUND((qty_curr_year - qty_prev_year) * 100.0 / qty_prev_year, 2)
                END AS growth_percent,
                ROUND(
                    qty_curr_year * (1 + NVL((qty_curr_year - qty_prev_year) / NULLIF(qty_prev_year, 0), 0))
                ) AS forecast_next_year,
                revenue_curr_year
            FROM yearly_stats
            ORDER BY growth_percent DESC, qty_curr_year DESC;
    END report_service_trends;

END PKG_ADMIN;