-- Триггер 1: Проверка корректности даты рождения пациента
CREATE OR REPLACE TRIGGER TRG_PATIENTS_BIRT_DATE
BEFORE INSERT OR UPDATE OF birth_date ON PATIENTS
FOR EACH ROW
BEGIN
    IF :NEW.birth_date > TRUNC(SYSDATE) THEN
        RAISE_APPLICATION_ERROR(-20001, 'Ошибка: Дата рождения пациента не может быть позже текущей даты.');
    END IF;
END;

-- Триггер 2: Контроль создания записи медкарты только для завершённого приёма (COMPLETED)
CREATE OR REPLACE TRIGGER TRG_MED_RECORDS_CHECK_APP
BEFORE INSERT ON MEDICAL_RECORDS
FOR EACH ROW
DECLARE
    v_status APPOINTMENTS.status%TYPE;
BEGIN
    SELECT status INTO v_status
    FROM APPOINTMENTS
    WHERE appointment_id = :NEW.appointment_id;

    IF v_status <> 'COMPLETED' THEN
        RAISE_APPLICATION_ERROR(-20002, 'Ошибка: Запись в медкарте можно создать только для приёма в статусе COMPLETED.');
    END IF;
END;


-- Проверочка
SELECT object_type, object_name, status 
FROM user_objects 
WHERE object_type IN ('TABLE', 'VIEW', 'TRIGGER')
ORDER BY object_type, object_name;