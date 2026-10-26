DROP TABLE TEACHER purge;

--создание таблицы нафиг
CREATE TABLE FACULTY (
    FACULTY CHAR(10) CONSTRAINT FACULTY_PK PRIMARY KEY,
    FACULTY_NAME VARCHAR2(50) DEFAULT '???'
);

CREATE TABLE PULPIT (
    PULPIT CHAR(20) CONSTRAINT PULPIT_PK PRIMARY KEY,
    PULPIT_NAME VARCHAR2(100),
    FACULTY CHAR(10) CONSTRAINT PULPIT_FACULTY_FK REFERENCES FACULTY(FACULTY)
);

CREATE TABLE TEACHER (
    TEACHER CHAR(10) CONSTRAINT TEACHER_PK PRIMARY KEY,
    TEACHER_NAME VARCHAR2(100),
    GENDER CHAR(2) CHECK (GENDER IN ('м', 'ж')),
    PULPIT CHAR(20) CONSTRAINT TEACHER_PULPIT_FK REFERENCES PULPIT(PULPIT)
);

CREATE TABLE SUBJECT (
    SUBJECT CHAR(10) CONSTRAINT SUBJECT_PK PRIMARY KEY,
    SUBJECT_NAME VARCHAR2(100) UNIQUE,
    PULPIT CHAR(20) CONSTRAINT SUBJECT_PULPIT_FK REFERENCES PULPIT(PULPIT)
);

CREATE TABLE AUDITORIUM_TYPE (
    AUDITORIUM_TYPE CHAR(10) CONSTRAINT AUDITORIUM_TYPE_PK PRIMARY KEY,
    AUDITORIUM_TYPENAME VARCHAR2(30)
);

CREATE TABLE AUDITORIUM (
    AUDITORIUM CHAR(20) CONSTRAINT AUDITORIUM_PK PRIMARY KEY,
    AUDITORIUM_NAME VARCHAR2(50),
    AUDITORIUM_TYPE CHAR(10) CONSTRAINT AUDITORIUM_AUDITORIUM_TYPE_FK REFERENCES AUDITORIUM_TYPE(AUDITORIUM_TYPE),
    AUDITORIUM_CAPACITY NUMBER DEFAULT 1 CHECK (AUDITORIUM_CAPACITY BETWEEN 1 AND 300)
);

INSERT INTO FACULTY (FACULTY, FACULTY_NAME) VALUES ('ИТ', 'Факульт информ технологий');
INSERT INTO PULPIT (PULPIT, PULPIT_NAME, FACULTY) VALUES ('ИСИТ', 'Информационных систем и технологий', 'ИТ');
INSERT INTO TEACHER (TEACHER, TEACHER_NAME, GENDER, PULPIT) VALUES ('СМЛВ', 'Смелов Владимир Владиславович', 'м', 'ИСИТ');
INSERT INTO SUBJECT (SUBJECT, SUBJECT_NAME, PULPIT) VALUES ('БД', 'Базы данных', 'ИСИТ');
INSERT INTO AUDITORIUM_TYPE (AUDITORIUM_TYPE, AUDITORIUM_TYPENAME) VALUES ('ЛК', 'Лекционная');
INSERT INTO AUDITORIUM (AUDITORIUM, AUDITORIUM_NAME, AUDITORIUM_TYPE, AUDITORIUM_CAPACITY) VALUES ('206-1', '206-1', 'ЛК', 15);
COMMIT;




-- 1
BEGIN
    NULL;
END;
/

-- 2
BEGIN
    DBMS_OUTPUT.PUT_LINE('Hello World!');
END;
/

-- 3
DECLARE
    v_num NUMBER;
BEGIN
    v_num := 10 / 0;
EXCEPTION
    WHEN OTHERS THEN
        DBMS_OUTPUT.PUT_LINE('Код ошибки: ' || SQLCODE);
        DBMS_OUTPUT.PUT_LINE('Сообщение: ' || SQLERRM);
END;
/

-- 4
DECLARE
    v_outer NUMBER := 1;
BEGIN
    DBMS_OUTPUT.PUT_LINE('Внешний блок');
    BEGIN
        DBMS_OUTPUT.PUT_LINE('Внутренний блок');
        v_outer := 1 / 0;
    EXCEPTION
        WHEN OTHERS THEN
            DBMS_OUTPUT.PUT_LINE('Ошибка перехвачена во внутреннем блоке');
    END;
    DBMS_OUTPUT.PUT_LINE('Продолжение внешнего блока');
END;
/


-- 5
SELECT * FROM v$parameter WHERE name = 'plsql_warnings';

-- 6 и 7
SELECT * FROM V$RESERVED_WORDS ORDER BY keyword;

-- 8
SELECT name, value FROM v$parameter WHERE name LIKE 'plsql%';


DECLARE
    -- 10-11
    v_int1 NUMBER(10) := 15;
    v_int2 NUMBER(10) := 4;
    v_mod NUMBER(10);
    
    -- 12-13
    v_fix1 NUMBER(5,2) := 123.45;
    v_fix2 NUMBER(5,-1) := 12345;
    
    -- 14-15
    v_bfloat BINARY_FLOAT := 1.5f;
    v_bdouble BINARY_DOUBLE := 2.5d;
    
    -- 16
    v_exp NUMBER := 1.5E3;
    
    -- 17
    v_bool BOOLEAN := TRUE;
    
    -- 18
    c_varchar CONSTANT VARCHAR2(20) := 'CONST_STR';
    c_char CONSTANT CHAR(5) := 'ABC  ';
    c_num CONSTANT NUMBER := 3.14;
BEGIN
    v_mod := MOD(v_int1, v_int2);
    DBMS_OUTPUT.PUT_LINE('Остаток от деления 15 на 4: ' || v_mod);
    DBMS_OUTPUT.PUT_LINE('Округление с отрицательным масштабом: ' || v_fix2);
    DBMS_OUTPUT.PUT_LINE('Экспонента: ' || v_exp);
    DBMS_OUTPUT.PUT_LINE('Работа с константами: ' || c_varchar || ' = ' || c_num);
END;
/

DECLARE
    -- 19
    v_teacher_name TEACHER.TEACHER_NAME%TYPE;
    
    -- 20
    v_faculty_rec FACULTY%ROWTYPE;
BEGIN
    SELECT TEACHER_NAME INTO v_teacher_name FROM TEACHER WHERE TEACHER = 'СМЛВ';
    DBMS_OUTPUT.PUT_LINE('Преподаватель (%TYPE): ' || v_teacher_name);

    SELECT * INTO v_faculty_rec FROM FACULTY WHERE FACULTY = 'ИТ';
    DBMS_OUTPUT.PUT_LINE('Факультет (%ROWTYPE): ' || v_faculty_rec.FACULTY_NAME);
END;
/



DECLARE
    v_counter NUMBER := 0;
    v_grade NUMBER := 5;
BEGIN
    -- 21-22
    IF v_grade = 5 THEN
        DBMS_OUTPUT.PUT_LINE('IF: Отлично');
    ELSIF v_grade = 4 THEN
        DBMS_OUTPUT.PUT_LINE('IF: Хорошо');
    ELSE
        DBMS_OUTPUT.PUT_LINE('IF: Удовлетворительно');
    END IF;

    -- 23
    CASE v_grade
        WHEN 5 THEN DBMS_OUTPUT.PUT_LINE('CASE: Отлично');
        WHEN 4 THEN DBMS_OUTPUT.PUT_LINE('CASE: Хорошо');
        ELSE DBMS_OUTPUT.PUT_LINE('CASE: Иное');
    END CASE;

    -- 24
    v_counter := 1;
    LOOP
        DBMS_OUTPUT.PUT_LINE('LOOP Итерация: ' || v_counter);
        v_counter := v_counter + 1;
        EXIT WHEN v_counter > 3;
    END LOOP;

    -- 25
    v_counter := 1;
    WHILE v_counter <= 3 LOOP
        DBMS_OUTPUT.PUT_LINE('WHILE Итерация: ' || v_counter);
        v_counter := v_counter + 1;
    END LOOP;

    -- 26
    FOR i IN 1..3 LOOP
        DBMS_OUTPUT.PUT_LINE('FOR Итерация: ' || i);
    END LOOP;
END;
/



