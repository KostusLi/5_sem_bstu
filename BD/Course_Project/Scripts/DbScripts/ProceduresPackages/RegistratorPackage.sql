CREATE OR REPLACE PACKAGE PKG_REGISTRAR AS
    -- 1. Регистрация учётной карточки нового пациента
    PROCEDURE register_patient(
        p_full_name     IN  VARCHAR2,
        p_birth_date    IN  DATE,
        p_phone         IN  VARCHAR2,
        p_policy_number IN  VARCHAR2,
        p_patient_id    OUT NUMBER
    );

    -- Поиск карточек пациентов по ФИО или номеру полиса
    PROCEDURE search_patients(
        p_search_query IN  VARCHAR2 DEFAULT NULL,
        p_cursor       OUT SYS_REFCURSOR
    );

    -- 2. Оформление записи пациента к врачу (резервирование свободного слота)
    PROCEDURE book_appointment(
        p_appointment_id IN NUMBER,
        p_patient_id     IN NUMBER
    );

    -- Отмена записи пациента (возврат слота в статус AVAILABLE)
    PROCEDURE cancel_appointment(p_appointment_id IN NUMBER);

    -- 3. Оформление счёта за оказанные медицинские услуги (по формуле 3.1)
    PROCEDURE create_invoice(
        p_appointment_id   IN  NUMBER,
        p_discount_percent IN  NUMBER DEFAULT 0,
        p_payment_id       OUT NUMBER
    );

    -- 4. Фиксация оплаты медицинского счёта
    PROCEDURE confirm_payment(p_payment_id IN NUMBER);

    -- Просмотр счетов и платежей
    PROCEDURE get_invoices(
        p_patient_id IN  NUMBER DEFAULT NULL,
        p_status     IN  VARCHAR2 DEFAULT NULL,
        p_cursor     OUT SYS_REFCURSOR
    );
END PKG_REGISTRAR;



CREATE OR REPLACE PACKAGE BODY PKG_REGISTRAR AS

    PROCEDURE register_patient(
        p_full_name     IN  VARCHAR2,
        p_birth_date    IN  DATE,
        p_phone         IN  VARCHAR2,
        p_policy_number IN  VARCHAR2,
        p_patient_id    OUT NUMBER
    ) IS
    BEGIN
        IF p_birth_date > TRUNC(SYSDATE) THEN
            RAISE_APPLICATION_ERROR(-20040, 'Дата рождения пациента не может быть позже текущей даты.');
        END IF;

        INSERT INTO PATIENTS (full_name, birth_date, phone, policy_number, registered_at)
        VALUES (TRIM(p_full_name), TRUNC(p_birth_date), p_phone, TRIM(p_policy_number), SYSDATE)
        RETURNING patient_id INTO p_patient_id;

        COMMIT;
    EXCEPTION
        WHEN DUP_VAL_ON_INDEX THEN
            RAISE_APPLICATION_ERROR(-20041, 'Пациент с таким номером полиса/паспорта уже зарегистрирован.');
    END register_patient;

    PROCEDURE search_patients(
        p_search_query IN  VARCHAR2 DEFAULT NULL,
        p_cursor       OUT SYS_REFCURSOR
    ) IS
    BEGIN
        OPEN p_cursor FOR
            SELECT patient_id, full_name, birth_date, phone, policy_number, registered_at
            FROM PATIENTS
            WHERE p_search_query IS NULL
               OR LOWER(full_name) LIKE '%' || LOWER(TRIM(p_search_query)) || '%'
               OR LOWER(policy_number) LIKE '%' || LOWER(TRIM(p_search_query)) || '%'
            ORDER BY full_name
            FETCH FIRST 100 ROWS ONLY;
    END search_patients;

    PROCEDURE book_appointment(
        p_appointment_id IN NUMBER,
        p_patient_id     IN NUMBER
    ) IS
        v_status APPOINTMENTS.status%TYPE;
        v_pat_id PATIENTS.patient_id%TYPE;
    BEGIN
        -- Проверяем существование пациента
        SELECT patient_id INTO v_pat_id
        FROM PATIENTS
        WHERE patient_id = p_patient_id;

        -- Блокируем строку слота для защиты от одновременной записи
        SELECT status INTO v_status
        FROM APPOINTMENTS
        WHERE appointment_id = p_appointment_id
        FOR UPDATE;

        IF v_status <> 'AVAILABLE' THEN
            ROLLBACK;
            RAISE_APPLICATION_ERROR(-20042, 'Выбранный слот уже занят или снят с расписания.');
        END IF;

        UPDATE APPOINTMENTS
        SET patient_id = p_patient_id,
            status     = 'BOOKED'
        WHERE appointment_id = p_appointment_id;

        COMMIT;
    EXCEPTION
        WHEN NO_DATA_FOUND THEN
            ROLLBACK;
            RAISE_APPLICATION_ERROR(-20043, 'Указанный слот расписания или пациент не найден.');
    END book_appointment;

    PROCEDURE cancel_appointment(p_appointment_id IN NUMBER) IS
        v_status APPOINTMENTS.status%TYPE;
    BEGIN
        SELECT status INTO v_status
        FROM APPOINTMENTS
        WHERE appointment_id = p_appointment_id
        FOR UPDATE;

        IF v_status <> 'BOOKED' THEN
            ROLLBACK;
            RAISE_APPLICATION_ERROR(-20044, 'Отменить можно только запись в статусе BOOKED.');
        END IF;

        UPDATE APPOINTMENTS
        SET patient_id = NULL,
            status     = 'AVAILABLE'
        WHERE appointment_id = p_appointment_id;

        COMMIT;
    EXCEPTION
        WHEN NO_DATA_FOUND THEN
            ROLLBACK;
            RAISE_APPLICATION_ERROR(-20043, 'Слот расписания не найден.');
    END cancel_appointment;

    PROCEDURE create_invoice(
        p_appointment_id   IN  NUMBER,
        p_discount_percent IN  NUMBER DEFAULT 0,
        p_payment_id       OUT NUMBER
    ) IS
        v_status       APPOINTMENTS.status%TYPE;
        v_total_amount NUMBER(10, 2);
    BEGIN
        SELECT status INTO v_status
        FROM APPOINTMENTS
        WHERE appointment_id = p_appointment_id;

        IF v_status <> 'COMPLETED' THEN
            RAISE_APPLICATION_ERROR(-20045, 'Счёт можно оформить только для завершённого приёма (COMPLETED).');
        END IF;

        -- Расчёт суммы по прайс-листу на момент приёма (формула 3.1)
        SELECT NVL(SUM(current_price * quantity), 0)
        INTO v_total_amount
        FROM APPOINTMENT_SERVICES
        WHERE appointment_id = p_appointment_id;

        IF v_total_amount <= 0 THEN
            RAISE_APPLICATION_ERROR(-20046, 'По данному приёму не зафиксировано платных услуг.');
        END IF;

        INSERT INTO PAYMENTS (appointment_id, amount, discount_percent, payment_date, status)
        VALUES (p_appointment_id, v_total_amount, NVL(p_discount_percent, 0), NULL, 'UNPAID')
        RETURNING payment_id INTO p_payment_id;

        COMMIT;
    EXCEPTION
        WHEN DUP_VAL_ON_INDEX THEN
            RAISE_APPLICATION_ERROR(-20047, 'Счёт по данному приёму уже оформлен.');
        WHEN NO_DATA_FOUND THEN
            RAISE_APPLICATION_ERROR(-20043, 'Приём не найден.');
    END create_invoice;

    PROCEDURE confirm_payment(p_payment_id IN NUMBER) IS
        v_status PAYMENTS.status%TYPE;
    BEGIN
        SELECT status INTO v_status
        FROM PAYMENTS
        WHERE payment_id = p_payment_id
        FOR UPDATE;

        IF v_status = 'PAID' THEN
            ROLLBACK;
            RAISE_APPLICATION_ERROR(-20048, 'Данный счёт уже оплачен.');
        END IF;

        UPDATE PAYMENTS
        SET status       = 'PAID',
            payment_date = SYSDATE
        WHERE payment_id = p_payment_id;

        COMMIT;
    EXCEPTION
        WHEN NO_DATA_FOUND THEN
            ROLLBACK;
            RAISE_APPLICATION_ERROR(-20049, 'Счёт не найден.');
    END confirm_payment;

    PROCEDURE get_invoices(
        p_patient_id IN  NUMBER DEFAULT NULL,
        p_status     IN  VARCHAR2 DEFAULT NULL,
        p_cursor     OUT SYS_REFCURSOR
    ) IS
    BEGIN
        OPEN p_cursor FOR
            SELECT payment_id, appointment_id, slot_start, patient_id, patient_name,
                   policy_number, doctor_name, specialty_name, amount,
                   discount_percent, final_amount, payment_date, payment_status
            FROM V_BILLING_DETAILS
            WHERE (p_patient_id IS NULL OR patient_id = p_patient_id)
              AND (p_status IS NULL OR payment_status = UPPER(TRIM(p_status)))
            ORDER BY slot_start DESC;
    END get_invoices;

END PKG_REGISTRAR;