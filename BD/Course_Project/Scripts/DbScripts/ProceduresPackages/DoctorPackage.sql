CREATE OR REPLACE PACKAGE PKG_DOCTOR AS
    -- 1. Просмотр персонального расписания врача
    PROCEDURE get_personal_schedule(
        p_doctor_id IN  NUMBER,
        p_date_from IN  DATE,
        p_date_to   IN  DATE,
        p_cursor    OUT SYS_REFCURSOR
    );

    -- 2. Фиксация оказанных услуг на приёме (с сохранением исторической цены из прайс-листа)
    PROCEDURE add_rendered_service(
        p_doctor_id      IN NUMBER,
        p_appointment_id IN NUMBER,
        p_service_id     IN NUMBER,
        p_quantity       IN NUMBER DEFAULT 1
    );

    -- 3. Завершение приёма и первичное заполнение записи в медицинской карте (включая JSON анализов)
    PROCEDURE complete_appointment(
        p_doctor_id      IN NUMBER,
        p_appointment_id IN NUMBER,
        p_complaints     IN VARCHAR2,
        p_diagnosis      IN VARCHAR2,
        p_prescriptions  IN VARCHAR2,
        p_analyses_json  IN CLOB DEFAULT NULL
    );

    -- 4. Просмотр электронной медицинской карты пациента (истории болезни)
    PROCEDURE get_patient_medical_card(
        p_patient_id IN  NUMBER,
        p_cursor     OUT SYS_REFCURSOR
    );

    -- 5. Обновление записи медицинской карты (разрешено только врачу, проводившему приём)
    PROCEDURE update_medical_record(
        p_doctor_id      IN NUMBER,
        p_appointment_id IN NUMBER,
        p_complaints     IN VARCHAR2,
        p_diagnosis      IN VARCHAR2,
        p_prescriptions  IN VARCHAR2,
        p_analyses_json  IN CLOB DEFAULT NULL
    );

    -- Просмотр услуг, оказанных в рамках конкретного приёма
    PROCEDURE get_appointment_services(
        p_appointment_id IN  NUMBER,
        p_cursor         OUT SYS_REFCURSOR
    );
END PKG_DOCTOR;



CREATE OR REPLACE PACKAGE BODY PKG_DOCTOR AS

    PROCEDURE get_personal_schedule(
        p_doctor_id IN  NUMBER,
        p_date_from IN  DATE,
        p_date_to   IN  DATE,
        p_cursor    OUT SYS_REFCURSOR
    ) IS
    BEGIN
        OPEN p_cursor FOR
            SELECT appointment_id, slot_start, cabinet_number, status,
                   patient_id, patient_name, policy_number
            FROM V_SCHEDULE_DETAILS
            WHERE doctor_id = p_doctor_id
              AND slot_start >= CAST(TRUNC(p_date_from) AS TIMESTAMP)
              AND slot_start <  CAST(TRUNC(p_date_to) + 1 AS TIMESTAMP)
            ORDER BY slot_start;
    END get_personal_schedule;

    PROCEDURE add_rendered_service(
        p_doctor_id      IN NUMBER,
        p_appointment_id IN NUMBER,
        p_service_id     IN NUMBER,
        p_quantity       IN NUMBER DEFAULT 1
    ) IS
        v_doc_id APPOINTMENTS.doctor_id%TYPE;
        v_status APPOINTMENTS.status%TYPE;
        v_price  SERVICES.price%TYPE;
    BEGIN
        SELECT doctor_id, status INTO v_doc_id, v_status
        FROM APPOINTMENTS
        WHERE appointment_id = p_appointment_id;

        IF v_doc_id <> p_doctor_id THEN
            RAISE_APPLICATION_ERROR(-20030, 'Вы не можете добавлять услуги в чужой приём.');
        END IF;

        IF v_status NOT IN ('BOOKED', 'COMPLETED') THEN
            RAISE_APPLICATION_ERROR(-20031, 'Фиксация услуг возможна только для записанного или завершённого приёма.');
        END IF;

        SELECT price INTO v_price
        FROM SERVICES
        WHERE service_id = p_service_id;

        MERGE INTO APPOINTMENT_SERVICES aps
        USING DUAL
        ON (aps.appointment_id = p_appointment_id AND aps.service_id = p_service_id)
        WHEN MATCHED THEN
            UPDATE SET aps.quantity = aps.quantity + p_quantity
        WHEN NOT MATCHED THEN
            INSERT (appointment_id, service_id, quantity, current_price)
            VALUES (p_appointment_id, p_service_id, p_quantity, v_price);

        COMMIT;
    EXCEPTION
        WHEN NO_DATA_FOUND THEN
            RAISE_APPLICATION_ERROR(-20032, 'Приём или услуга не найдены.');
    END add_rendered_service;

    PROCEDURE complete_appointment(
        p_doctor_id      IN NUMBER,
        p_appointment_id IN NUMBER,
        p_complaints     IN VARCHAR2,
        p_diagnosis      IN VARCHAR2,
        p_prescriptions  IN VARCHAR2,
        p_analyses_json  IN CLOB DEFAULT NULL
    ) IS
        v_doc_id       APPOINTMENTS.doctor_id%TYPE;
        v_status       APPOINTMENTS.status%TYPE;
        v_services_cnt NUMBER;
    BEGIN
        SELECT doctor_id, status INTO v_doc_id, v_status
        FROM APPOINTMENTS
        WHERE appointment_id = p_appointment_id
        FOR UPDATE;

        IF v_doc_id <> p_doctor_id THEN
            RAISE_APPLICATION_ERROR(-20033, 'Завершить приём может только врач, за которым закреплён слот.');
        END IF;

        IF v_status <> 'BOOKED' THEN
            RAISE_APPLICATION_ERROR(-20034, 'Завершить можно только приём в статусе BOOKED.');
        END IF;

        SELECT COUNT(*) INTO v_services_cnt
        FROM APPOINTMENT_SERVICES
        WHERE appointment_id = p_appointment_id;

        IF v_services_cnt = 0 THEN
            RAISE_APPLICATION_ERROR(-20035, 'Перед завершением приёма необходимо зафиксировать хотя бы одну оказанную услугу.');
        END IF;

        UPDATE APPOINTMENTS
        SET status = 'COMPLETED'
        WHERE appointment_id = p_appointment_id;

        INSERT INTO MEDICAL_RECORDS (appointment_id, complaints, diagnosis, prescriptions, analyses_results)
        VALUES (
            p_appointment_id,
            p_complaints,
            p_diagnosis,
            p_prescriptions,
            CASE WHEN p_analyses_json IS NOT NULL AND LENGTH(TRIM(p_analyses_json)) > 0 
                 THEN JSON(p_analyses_json) 
                 ELSE NULL 
            END
        );

        COMMIT;
    EXCEPTION
        WHEN NO_DATA_FOUND THEN
            RAISE_APPLICATION_ERROR(-20032, 'Приём не найден.');
    END complete_appointment;

    PROCEDURE get_patient_medical_card(
        p_patient_id IN  NUMBER,
        p_cursor     OUT SYS_REFCURSOR
    ) IS
    BEGIN
        OPEN p_cursor FOR
            SELECT record_id, appointment_id, visit_date, patient_id, patient_name,
                   birth_date, policy_number, doctor_id, doctor_name, specialty_name,
                   complaints, diagnosis, prescriptions, analyses_json
            FROM V_PATIENT_MEDICAL_CARD
            WHERE patient_id = p_patient_id
            ORDER BY visit_date DESC;
    END get_patient_medical_card;

    PROCEDURE update_medical_record(
        p_doctor_id      IN NUMBER,
        p_appointment_id IN NUMBER,
        p_complaints     IN VARCHAR2,
        p_diagnosis      IN VARCHAR2,
        p_prescriptions  IN VARCHAR2,
        p_analyses_json  IN CLOB DEFAULT NULL
    ) IS
        v_doc_id APPOINTMENTS.doctor_id%TYPE;
        v_status APPOINTMENTS.status%TYPE;
    BEGIN
        SELECT doctor_id, status INTO v_doc_id, v_status
        FROM APPOINTMENTS
        WHERE appointment_id = p_appointment_id;

        IF v_doc_id <> p_doctor_id THEN
            RAISE_APPLICATION_ERROR(-20036, 'Обновлять медицинскую карту может только врач, проводивший данный приём.');
        END IF;

        IF v_status <> 'COMPLETED' THEN
            RAISE_APPLICATION_ERROR(-20037, 'Запись в медкарте доступна только для приёма в статусе COMPLETED.');
        END IF;

        UPDATE MEDICAL_RECORDS
        SET complaints       = p_complaints,
            diagnosis        = p_diagnosis,
            prescriptions    = p_prescriptions,
            analyses_results = CASE WHEN p_analyses_json IS NOT NULL AND LENGTH(TRIM(p_analyses_json)) > 0 
                                    THEN JSON(p_analyses_json) 
                                    ELSE analyses_results 
                               END
        WHERE appointment_id = p_appointment_id;

        IF SQL%ROWCOUNT = 0 THEN
            INSERT INTO MEDICAL_RECORDS (appointment_id, complaints, diagnosis, prescriptions, analyses_results)
            VALUES (
                p_appointment_id,
                p_complaints,
                p_diagnosis,
                p_prescriptions,
                CASE WHEN p_analyses_json IS NOT NULL AND LENGTH(TRIM(p_analyses_json)) > 0 
                     THEN JSON(p_analyses_json) 
                     ELSE NULL 
                END
            );
        END IF;

        COMMIT;
    END update_medical_record;

    PROCEDURE get_appointment_services(
        p_appointment_id IN  NUMBER,
        p_cursor         OUT SYS_REFCURSOR
    ) IS
    BEGIN
        OPEN p_cursor FOR
            SELECT aps.appointment_service_id, aps.appointment_id, aps.service_id,
                   s.service_name, aps.quantity, aps.current_price,
                   (aps.quantity * aps.current_price) AS line_total
            FROM APPOINTMENT_SERVICES aps
            JOIN SERVICES s ON aps.service_id = s.service_id
            WHERE aps.appointment_id = p_appointment_id
            ORDER BY aps.appointment_service_id;
    END get_appointment_services;

END PKG_DOCTOR;