--1
ALTER TABLE TEACHER ADD (BIRTHDAY DATE, SALARY NUMBER);
UPDATE TEACHER SET BIRTHDAY = TO_DATE('1985-06-15', 'YYYY-MM-DD'), SALARY = 1500 WHERE TEACHER = 'СМЛВ';
INSERT INTO TEACHER (TEACHER, TEACHER_NAME, GENDER, PULPIT, BIRTHDAY, SALARY) 
VALUES ('ИВНВ', 'Иванов Иван Иванович', 'м', 'ИСИТ', TO_DATE('1990-12-05', 'YYYY-MM-DD'), 1200);
COMMIT;

--2
SELECT 
    SUBSTR(TEACHER_NAME, 1, INSTR(TEACHER_NAME, ' ') - 1) || ' ' || 
    SUBSTR(TEACHER_NAME, INSTR(TEACHER_NAME, ' ') + 1, 1) || '.' || 
    SUBSTR(TEACHER_NAME, INSTR(TEACHER_NAME, ' ', 1, 2) + 1, 1) || '.' AS SHORT_NAME
FROM TEACHER;

--3
SELECT TEACHER_NAME, BIRTHDAY 
FROM TEACHER 
WHERE TO_CHAR(BIRTHDAY, 'D') = '1';

--4	
CREATE OR REPLACE VIEW V_TEACHERS_NEXT_MONTH AS
SELECT TEACHER_NAME, BIRTHDAY
FROM TEACHER
WHERE EXTRACT(MONTH FROM BIRTHDAY) = MOD(EXTRACT(MONTH FROM SYSDATE), 12) + 1;

--5
CREATE OR REPLACE VIEW V_TEACHERS_BIRTH_MONTH_COUNT AS
SELECT EXTRACT(MONTH FROM BIRTHDAY) AS BIRTH_MONTH, COUNT(TEACHER) AS TEACHER_COUNT
FROM TEACHER
WHERE BIRTHDAY IS NOT NULL
GROUP BY EXTRACT(MONTH FROM BIRTHDAY);

--6
SET SERVEROUTPUT ON;
DECLARE
    CURSOR c_anniversary IS
        SELECT TEACHER_NAME, 
               (EXTRACT(YEAR FROM SYSDATE) + 1 - EXTRACT(YEAR FROM BIRTHDAY)) AS NEXT_AGE
        FROM TEACHER
        WHERE BIRTHDAY IS NOT NULL 
          AND MOD(EXTRACT(YEAR FROM SYSDATE) + 1 - EXTRACT(YEAR FROM BIRTHDAY), 5) = 0;
BEGIN
    DBMS_OUTPUT.PUT_LINE('Юбиляры следующего года:');
    FOR r IN c_anniversary LOOP
        DBMS_OUTPUT.PUT_LINE(r.TEACHER_NAME || ' исполняется ' || r.NEXT_AGE);
    END LOOP;
END;
/

--7
DECLARE
    CURSOR c_salary IS
        SELECT f.FACULTY, 
               p.PULPIT, 
               FLOOR(AVG(t.SALARY)) AS AVG_SAL
        FROM FACULTY f
        JOIN PULPIT p ON f.FACULTY = p.FACULTY
        JOIN TEACHER t ON p.PULPIT = t.PULPIT
        GROUP BY ROLLUP(f.FACULTY, p.PULPIT);
BEGIN
    FOR r IN c_salary LOOP
        IF r.FACULTY IS NULL THEN
            DBMS_OUTPUT.PUT_LINE('ИТОГО по всем факультетам: ' || r.AVG_SAL);
        ELSIF r.PULPIT IS NULL THEN
            DBMS_OUTPUT.PUT_LINE('Итого по факультету ' || r.FACULTY || ': ' || r.AVG_SAL);
        ELSE
            DBMS_OUTPUT.PUT_LINE('Кафедра ' || r.PULPIT || ' (Факультет ' || r.FACULTY || '): ' || r.AVG_SAL);
        END IF;
    END LOOP;
END;
/

--8
DECLARE
    TYPE AddressRecord IS RECORD (
        City VARCHAR2(50),
        Street VARCHAR2(50)
    );
    TYPE TeacherInfoRecord IS RECORD (
        FIO VARCHAR2(100),
        Addr AddressRecord
    );
    rec1 TeacherInfoRecord;
    rec2 TeacherInfoRecord;
BEGIN
    rec1.FIO := 'Смелов Владимир Владиславович';
    rec1.Addr.City := 'Минск';
    rec1.Addr.Street := 'Свердлова 13а';
    rec2 := rec1; 
    DBMS_OUTPUT.PUT_LINE('Оригинал: ' || rec1.FIO || ' из г. ' || rec1.Addr.City);
    DBMS_OUTPUT.PUT_LINE('Копия: ' || rec2.FIO || ', ул. ' || rec2.Addr.Street);
END;
/