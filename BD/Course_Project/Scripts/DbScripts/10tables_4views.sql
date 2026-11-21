
-- 1. Таблица ролей пользователей (Таблица 3.2)
CREATE TABLE ROLES (
    role_id   NUMBER(3) GENERATED ALWAYS AS IDENTITY,
    role_name VARCHAR2(30) NOT NULL,
    CONSTRAINT pk_roles PRIMARY KEY (role_id) USING INDEX TABLESPACE TS_CLINIC_IDX,
    CONSTRAINT uq_roles_name UNIQUE (role_name) USING INDEX TABLESPACE TS_CLINIC_IDX,
    CONSTRAINT chk_roles_name CHECK (role_name IN ('ADMIN', 'DOCTOR', 'REGISTRAR'))
) TABLESPACE TS_CLINIC_DATA;

-- 2. Таблица учётных записей сотрудников клиники (Таблица 3.3)
CREATE TABLE USERS (
    user_id       NUMBER(10) GENERATED ALWAYS AS IDENTITY,
    role_id       NUMBER(3) NOT NULL,
    login         VARCHAR2(50) NOT NULL,
    password_hash VARCHAR2(128) NOT NULL,
    full_name     VARCHAR2(150) NOT NULL,
    phone         VARCHAR2(20),
    is_active     NUMBER(1) DEFAULT 1 NOT NULL,
    CONSTRAINT pk_users PRIMARY KEY (user_id) USING INDEX TABLESPACE TS_CLINIC_IDX,
    CONSTRAINT fk_users_role FOREIGN KEY (role_id) REFERENCES ROLES (role_id),
    CONSTRAINT uq_users_login UNIQUE (login) USING INDEX TABLESPACE TS_CLINIC_IDX,
    CONSTRAINT chk_users_active CHECK (is_active IN (0, 1))
) TABLESPACE TS_CLINIC_DATA;

-- 3. Таблица медицинских специальностей (Таблица 3.4)
CREATE TABLE SPECIALTIES (
    specialty_id   NUMBER(5) GENERATED ALWAYS AS IDENTITY,
    specialty_name VARCHAR2(100) NOT NULL,
    CONSTRAINT pk_specialties PRIMARY KEY (specialty_id) USING INDEX TABLESPACE TS_CLINIC_IDX,
    CONSTRAINT uq_specialties_name UNIQUE (specialty_name) USING INDEX TABLESPACE TS_CLINIC_IDX
) TABLESPACE TS_CLINIC_DATA;

-- 4. Таблица врачей клиники (Таблица 3.5)
CREATE TABLE DOCTORS (
    doctor_id    NUMBER(10) GENERATED ALWAYS AS IDENTITY,
    user_id      NUMBER(10) NOT NULL,
    specialty_id NUMBER(5) NOT NULL,
    CONSTRAINT pk_doctors PRIMARY KEY (doctor_id) USING INDEX TABLESPACE TS_CLINIC_IDX,
    CONSTRAINT uq_doctors_user UNIQUE (user_id) USING INDEX TABLESPACE TS_CLINIC_IDX,
    CONSTRAINT fk_doctors_user FOREIGN KEY (user_id) REFERENCES USERS (user_id),
    CONSTRAINT fk_doctors_specialty FOREIGN KEY (specialty_id) REFERENCES SPECIALTIES (specialty_id)
) TABLESPACE TS_CLINIC_DATA;

-- 5. Таблица справочника медицинских услуг / прайс-лист (Таблица 3.6 + 3.12)
CREATE TABLE SERVICES (
    service_id   NUMBER(10) GENERATED ALWAYS AS IDENTITY,
    specialty_id NUMBER(5) NOT NULL,
    service_name VARCHAR2(200) NOT NULL,
    price        NUMBER(10, 2) NOT NULL,
    CONSTRAINT pk_services PRIMARY KEY (service_id) USING INDEX TABLESPACE TS_CLINIC_IDX,
    CONSTRAINT fk_services_specialty FOREIGN KEY (specialty_id) REFERENCES SPECIALTIES (specialty_id),
    CONSTRAINT uq_services_spec_name UNIQUE (specialty_id, service_name) USING INDEX TABLESPACE TS_CLINIC_IDX,
    CONSTRAINT chk_services_price CHECK (price >= 0)
) TABLESPACE TS_CLINIC_DATA;

-- 6. Таблица карточек пациентов (Таблица 3.7)
CREATE TABLE PATIENTS (
    patient_id    NUMBER(10) GENERATED ALWAYS AS IDENTITY,
    full_name     VARCHAR2(150) NOT NULL,
    birth_date    DATE NOT NULL,
    phone         VARCHAR2(20),
    policy_number VARCHAR2(30) NOT NULL,
    registered_at DATE DEFAULT SYSDATE NOT NULL,
    CONSTRAINT pk_patients PRIMARY KEY (patient_id) USING INDEX TABLESPACE TS_CLINIC_IDX,
    CONSTRAINT uq_patients_policy UNIQUE (policy_number) USING INDEX TABLESPACE TS_CLINIC_IDX
) TABLESPACE TS_CLINIC_DATA;

-- 7. Таблица расписания (слотов) и приёмов (Таблица 3.8 + 3.12)
CREATE TABLE APPOINTMENTS (
    appointment_id NUMBER(10) GENERATED ALWAYS AS IDENTITY,
    doctor_id      NUMBER(10) NOT NULL,
    patient_id     NUMBER(10),
    slot_start     TIMESTAMP(0) NOT NULL,
    cabinet_number NUMBER(3) NOT NULL,
    status         VARCHAR2(10) DEFAULT 'AVAILABLE' NOT NULL,
    CONSTRAINT pk_appointments PRIMARY KEY (appointment_id) USING INDEX TABLESPACE TS_CLINIC_IDX,
    CONSTRAINT fk_appointments_doctor FOREIGN KEY (doctor_id) REFERENCES DOCTORS (doctor_id),
    CONSTRAINT fk_appointments_patient FOREIGN KEY (patient_id) REFERENCES PATIENTS (patient_id),
    CONSTRAINT uq_appointments_doc_slot UNIQUE (doctor_id, slot_start) USING INDEX TABLESPACE TS_CLINIC_IDX,
    CONSTRAINT chk_appointments_cabinet CHECK (cabinet_number > 0),
    CONSTRAINT chk_appointments_status CHECK (status IN ('AVAILABLE', 'BOOKED', 'COMPLETED', 'CANCELLED')),
    CONSTRAINT chk_appointments_patient_state CHECK (
        (status IN ('AVAILABLE', 'CANCELLED') AND patient_id IS NULL) OR
        (status IN ('BOOKED', 'COMPLETED') AND patient_id IS NOT NULL)
    )
) TABLESPACE TS_CLINIC_DATA;

-- 8. Таблица оказанных в рамках приёма услуг (Таблица 3.9 + 3.12)
CREATE TABLE APPOINTMENT_SERVICES (
    appointment_service_id NUMBER(10) GENERATED ALWAYS AS IDENTITY,
    appointment_id         NUMBER(10) NOT NULL,
    service_id             NUMBER(10) NOT NULL,
    quantity               NUMBER(3) DEFAULT 1 NOT NULL,
    current_price          NUMBER(10, 2) NOT NULL,
    CONSTRAINT pk_app_services PRIMARY KEY (appointment_service_id) USING INDEX TABLESPACE TS_CLINIC_IDX,
    CONSTRAINT fk_app_services_app FOREIGN KEY (appointment_id) REFERENCES APPOINTMENTS (appointment_id),
    CONSTRAINT fk_app_services_srv FOREIGN KEY (service_id) REFERENCES SERVICES (service_id),
    CONSTRAINT uq_app_services_pair UNIQUE (appointment_id, service_id) USING INDEX TABLESPACE TS_CLINIC_IDX,
    CONSTRAINT chk_app_services_qty CHECK (quantity > 0),
    CONSTRAINT chk_app_services_price CHECK (current_price >= 0)
) TABLESPACE TS_CLINIC_DATA;

-- 9. Таблица записей электронной медицинской карты с типом JSON (Таблица 3.10)
CREATE TABLE MEDICAL_RECORDS (
    record_id        NUMBER(10) GENERATED ALWAYS AS IDENTITY,
    appointment_id   NUMBER(10) NOT NULL,
    complaints       VARCHAR2(4000),
    diagnosis        VARCHAR2(1000),
    prescriptions    VARCHAR2(4000),
    analyses_results JSON,
    CONSTRAINT pk_medical_records PRIMARY KEY (record_id) USING INDEX TABLESPACE TS_CLINIC_IDX,
    CONSTRAINT uq_medical_records_app UNIQUE (appointment_id) USING INDEX TABLESPACE TS_CLINIC_IDX,
    CONSTRAINT fk_medical_records_app FOREIGN KEY (appointment_id) REFERENCES APPOINTMENTS (appointment_id)
) TABLESPACE TS_CLINIC_DATA;

-- 10. Таблица счетов и платежей с виртуальным столбцом (Таблица 3.11 + 3.12)
CREATE TABLE PAYMENTS (
    payment_id       NUMBER(10) GENERATED ALWAYS AS IDENTITY,
    appointment_id   NUMBER(10) NOT NULL,
    amount           NUMBER(10, 2) NOT NULL,
    discount_percent NUMBER(5, 2) DEFAULT 0 NOT NULL,
    final_amount     NUMBER(10, 2) GENERATED ALWAYS AS (ROUND(amount - amount * discount_percent / 100, 2)) VIRTUAL,
    payment_date     DATE,
    status           VARCHAR2(10) DEFAULT 'UNPAID' NOT NULL,
    CONSTRAINT pk_payments PRIMARY KEY (payment_id) USING INDEX TABLESPACE TS_CLINIC_IDX,
    CONSTRAINT uq_payments_app UNIQUE (appointment_id) USING INDEX TABLESPACE TS_CLINIC_IDX,
    CONSTRAINT fk_payments_app FOREIGN KEY (appointment_id) REFERENCES APPOINTMENTS (appointment_id),
    CONSTRAINT chk_payments_amount CHECK (amount >= 0),
    CONSTRAINT chk_payments_discount CHECK (discount_percent BETWEEN 0 AND 100),
    CONSTRAINT chk_payments_status CHECK (status IN ('UNPAID', 'PAID')),
    CONSTRAINT chk_payments_date_state CHECK (
        (status = 'UNPAID' AND payment_date IS NULL) OR
        (status = 'PAID' AND payment_date IS NOT NULL)
    )
) TABLESPACE TS_CLINIC_DATA;

-- ============================================================================
-- ПРЕДСТАВЛЕНИЯ (VIEWS) ДЛЯ УПРОЩЕНИЯ ДОСТУПА ИЗ ПРОЦЕДУР
-- ============================================================================

-- 1. Представление актуального прайс-листа клиники
CREATE OR REPLACE VIEW V_PRICELIST AS
SELECT 
    s.service_id,
    sp.specialty_id,
    sp.specialty_name,
    s.service_name,
    s.price
FROM SERVICES s
JOIN SPECIALTIES sp ON s.specialty_id = sp.specialty_id;

-- 2. Представление общего расписания врачей
CREATE OR REPLACE VIEW V_SCHEDULE_DETAILS AS
SELECT 
    a.appointment_id,
    a.slot_start,
    a.cabinet_number,
    a.status,
    d.doctor_id,
    u.full_name AS doctor_name,
    u.is_active AS doctor_active,
    sp.specialty_id,
    sp.specialty_name,
    p.patient_id,
    p.full_name AS patient_name,
    p.policy_number
FROM APPOINTMENTS a
JOIN DOCTORS d ON a.doctor_id = d.doctor_id
JOIN USERS u ON d.user_id = u.user_id
JOIN SPECIALTIES sp ON d.specialty_id = sp.specialty_id
LEFT JOIN PATIENTS p ON a.patient_id = p.patient_id;

-- 3. Представление электронной медицинской карты пациента
CREATE OR REPLACE VIEW V_PATIENT_MEDICAL_CARD AS
SELECT 
    mr.record_id,
    a.appointment_id,
    a.slot_start AS visit_date,
    p.patient_id,
    p.full_name AS patient_name,
    p.birth_date,
    p.policy_number,
    d.doctor_id,
    u.full_name AS doctor_name,
    sp.specialty_name,
    mr.complaints,
    mr.diagnosis,
    mr.prescriptions,
    JSON_SERIALIZE(mr.analyses_results RETURNING CLOB PRETTY) AS analyses_json
FROM MEDICAL_RECORDS mr
JOIN APPOINTMENTS a ON mr.appointment_id = a.appointment_id
JOIN PATIENTS p ON a.patient_id = p.patient_id
JOIN DOCTORS d ON a.doctor_id = d.doctor_id
JOIN USERS u ON d.user_id = u.user_id
JOIN SPECIALTIES sp ON d.specialty_id = sp.specialty_id;

-- 4. Представление детализации счетов и оплат
CREATE OR REPLACE VIEW V_BILLING_DETAILS AS
SELECT 
    pay.payment_id,
    pay.appointment_id,
    a.slot_start,
    p.patient_id,
    p.full_name AS patient_name,
    p.policy_number,
    u.full_name AS doctor_name,
    sp.specialty_name,
    pay.amount,
    pay.discount_percent,
    pay.final_amount,
    pay.payment_date,
    pay.status AS payment_status
FROM PAYMENTS pay
JOIN APPOINTMENTS a ON pay.appointment_id = a.appointment_id
JOIN PATIENTS p ON a.patient_id = p.patient_id
JOIN DOCTORS d ON a.doctor_id = d.doctor_id
JOIN USERS u ON d.user_id = u.user_id
JOIN SPECIALTIES sp ON d.specialty_id = sp.specialty_id;