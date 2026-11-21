DECLARE
    v_name TEACHER.TEACHER_NAME%TYPE;
    v_dummy NUMBER;
BEGIN
    -- 1
    SELECT TEACHER_NAME INTO v_name FROM TEACHER WHERE TEACHER = 'СМЛВ';
    DBMS_OUTPUT.PUT_LINE('1. Точная выборка: ' || v_name);

    -- 2-3
    BEGIN
        SELECT 1 INTO v_dummy FROM ALL_OBJECTS FETCH FIRST 2 ROWS ONLY;
    EXCEPTION
        WHEN TOO_MANY_ROWS THEN
            DBMS_OUTPUT.PUT_LINE('3. Поймано исключение TOO_MANY_ROWS (Неточная выборка)');
        WHEN OTHERS THEN
            DBMS_OUTPUT.PUT_LINE('2. Ошибка: ' || SQLCODE || ' - ' || SQLERRM);
    END;

    -- 4
    BEGIN
        SELECT TEACHER_NAME INTO v_name FROM TEACHER WHERE TEACHER = 'НЕТ_ТАКОГО';
    EXCEPTION
        WHEN NO_DATA_FOUND THEN
            DBMS_OUTPUT.PUT_LINE('4. Поймано исключение NO_DATA_FOUND');
    END;

    UPDATE AUDITORIUM SET AUDITORIUM_CAPACITY = AUDITORIUM_CAPACITY WHERE 1=0;
    DBMS_OUTPUT.PUT_LINE('4. Атрибут SQL%ROWCOUNT после UPDATE: ' || SQL%ROWCOUNT);
END;



BEGIN
    -- 5
    UPDATE AUDITORIUM SET AUDITORIUM_CAPACITY = 20 WHERE AUDITORIUM = '206-1';
    DBMS_OUTPUT.PUT_LINE('5. Обновлено строк: ' || SQL%ROWCOUNT);
    ROLLBACK;

    -- 6.
    BEGIN
        UPDATE TEACHER SET PULPIT = 'НЕ_СУЩ' WHERE TEACHER = 'СМЛВ';
    EXCEPTION
        WHEN OTHERS THEN DBMS_OUTPUT.PUT_LINE('6. Ошибка целостности UPDATE: ' || SQLERRM);
    END;

    -- 7
    INSERT INTO AUDITORIUM (AUDITORIUM, AUDITORIUM_NAME, AUDITORIUM_TYPE, AUDITORIUM_CAPACITY) 
    VALUES ('101-1', '101-1', 'ЛК', 30);
    DBMS_OUTPUT.PUT_LINE('7. Добавлено строк: ' || SQL%ROWCOUNT);
    ROLLBACK;

    -- 8
    BEGIN
        INSERT INTO TEACHER (TEACHER, PULPIT) VALUES ('ТЕСТ', 'НЕ_СУЩ');
    EXCEPTION
        WHEN OTHERS THEN DBMS_OUTPUT.PUT_LINE('8. Ошибка целостности INSERT: ' || SQLERRM);
    END;

    -- 9
    DELETE FROM AUDITORIUM WHERE AUDITORIUM = '206-1';
    DBMS_OUTPUT.PUT_LINE('9. Удалено строк: ' || SQL%ROWCOUNT);
    ROLLBACK;

    -- 10
    BEGIN
        DELETE FROM PULPIT WHERE PULPIT = 'ИСИТ';
    EXCEPTION
        WHEN OTHERS THEN DBMS_OUTPUT.PUT_LINE('10. Ошибка целостности DELETE: ' || SQLERRM);
    END;
END;




DECLARE
    -- 11
    CURSOR c_teacher IS SELECT TEACHER, TEACHER_NAME FROM TEACHER;
    v_t_id TEACHER.TEACHER%TYPE;
    v_t_name TEACHER.TEACHER_NAME%TYPE;

    -- 12
    CURSOR c_subj IS SELECT * FROM SUBJECT;
    rec_subj c_subj%ROWTYPE;
BEGIN
    DBMS_OUTPUT.PUT_LINE('11. LOOP цикл (%TYPE)');
    OPEN c_teacher;
    LOOP
        FETCH c_teacher INTO v_t_id, v_t_name;
        EXIT WHEN c_teacher%NOTFOUND;
        DBMS_OUTPUT.PUT_LINE(v_t_id || ' - ' || v_t_name);
    END LOOP;
    CLOSE c_teacher;

    DBMS_OUTPUT.PUT_LINE('12. WHILE цикл (%ROWTYPE)');
    OPEN c_subj;
    FETCH c_subj INTO rec_subj;
    WHILE c_subj%FOUND LOOP
        DBMS_OUTPUT.PUT_LINE(rec_subj.SUBJECT || ' - ' || rec_subj.SUBJECT_NAME);
        FETCH c_subj INTO rec_subj;
    END LOOP;
    CLOSE c_subj;

    DBMS_OUTPUT.PUT_LINE('13. FOR цикл с JOIN');
    FOR rec IN (
        SELECT p.PULPIT_NAME, t.TEACHER_NAME 
        FROM PULPIT p JOIN TEACHER t ON p.PULPIT = t.PULPIT
    ) LOOP
        DBMS_OUTPUT.PUT_LINE(rec.PULPIT_NAME || ': ' || rec.TEACHER_NAME);
    END LOOP;
END;



DECLARE
    -- 14
    CURSOR c_aud(p_min NUMBER, p_max NUMBER) IS 
        SELECT AUDITORIUM_NAME, AUDITORIUM_CAPACITY 
        FROM AUDITORIUM 
        WHERE AUDITORIUM_CAPACITY BETWEEN p_min AND p_max;
    v_a_name AUDITORIUM.AUDITORIUM_NAME%TYPE;
    v_a_cap AUDITORIUM.AUDITORIUM_CAPACITY%TYPE;
    
    -- 15
    TYPE ref_cur IS REF CURSOR;
    rc ref_cur; 
    
    -- 16
    CURSOR c_sub IS 
        SELECT PULPIT_NAME, 
               CURSOR(SELECT TEACHER_NAME FROM TEACHER t WHERE t.PULPIT = p.PULPIT) as t_cur
        FROM PULPIT p;
    v_pulpit_name PULPIT.PULPIT_NAME%TYPE;
    v_t_cur SYS_REFCURSOR;
    v_t_name TEACHER.TEACHER_NAME%TYPE;
BEGIN
    DBMS_OUTPUT.PUT_LINE('14. Три цикла для аудиторий');
    OPEN c_aud(0, 19);
    LOOP
        FETCH c_aud INTO v_a_name, v_a_cap;
        EXIT WHEN c_aud%NOTFOUND;
        DBMS_OUTPUT.PUT_LINE('<20: ' || v_a_name || ' (' || v_a_cap || ')');
    END LOOP;
    CLOSE c_aud;
    
    OPEN c_aud(21, 30);
    FETCH c_aud INTO v_a_name, v_a_cap;
    WHILE c_aud%FOUND LOOP
        DBMS_OUTPUT.PUT_LINE('21-30: ' || v_a_name || ' (' || v_a_cap || ')');
        FETCH c_aud INTO v_a_name, v_a_cap;
    END LOOP;
    CLOSE c_aud;

    FOR rec IN c_aud(31, 60) LOOP
        DBMS_OUTPUT.PUT_LINE('31-60: ' || rec.AUDITORIUM_NAME || ' (' || rec.AUDITORIUM_CAPACITY || ')');
    END LOOP;

    DBMS_OUTPUT.PUT_LINE('15. SYS_REFCURSOR');
    OPEN rc FOR SELECT AUDITORIUM_NAME FROM AUDITORIUM WHERE AUDITORIUM_CAPACITY > 80;
    LOOP
        FETCH rc INTO v_a_name;
        EXIT WHEN rc%NOTFOUND;
        DBMS_OUTPUT.PUT_LINE('> 80: ' || v_a_name);
    END LOOP;
    CLOSE rc;

    DBMS_OUTPUT.PUT_LINE('16. Курсорный подзапрос');
    OPEN c_sub;
    LOOP
        FETCH c_sub INTO v_pulpit_name, v_t_cur;
        EXIT WHEN c_sub%NOTFOUND;
        DBMS_OUTPUT.PUT_LINE('Кафедра: ' || v_pulpit_name);
        LOOP
            FETCH v_t_cur INTO v_t_name;
            EXIT WHEN v_t_cur%NOTFOUND;
            DBMS_OUTPUT.PUT_LINE('  - ' || v_t_name);
        END LOOP;
    END LOOP;
    CLOSE c_sub;
END;




DECLARE
    -- 17
    CURSOR c_aud_upd(p_min NUMBER, p_max NUMBER) IS 
        SELECT AUDITORIUM_CAPACITY FROM AUDITORIUM 
        WHERE AUDITORIUM_CAPACITY BETWEEN p_min AND p_max FOR UPDATE;
        
    -- 18
    CURSOR c_aud_del(p_min NUMBER, p_max NUMBER) IS 
        SELECT AUDITORIUM_CAPACITY FROM AUDITORIUM 
        WHERE AUDITORIUM_CAPACITY BETWEEN p_min AND p_max FOR UPDATE;
    v_cap AUDITORIUM.AUDITORIUM_CAPACITY%TYPE;
    
    -- 19
    v_rowid ROWID;
    
    -- 20
    v_counter NUMBER := 0;
BEGIN
    -- 17
    FOR rec IN c_aud_upd(40, 80) LOOP
        UPDATE AUDITORIUM SET AUDITORIUM_CAPACITY = AUDITORIUM_CAPACITY * 0.9 WHERE CURRENT OF c_aud_upd;
    END LOOP;
    
    -- 18
    OPEN c_aud_del(0, 20);
    FETCH c_aud_del INTO v_cap;
    WHILE c_aud_del%FOUND LOOP
        DELETE FROM AUDITORIUM WHERE CURRENT OF c_aud_del;
        FETCH c_aud_del INTO v_cap;
    END LOOP;
    CLOSE c_aud_del;
    
    ROLLBACK;
    
    -- 19
    SELECT MIN(ROWID) INTO v_rowid FROM AUDITORIUM;
    IF v_rowid IS NOT NULL THEN
        UPDATE AUDITORIUM SET AUDITORIUM_CAPACITY = AUDITORIUM_CAPACITY + 1 WHERE ROWID = v_rowid;
        DBMS_OUTPUT.PUT_LINE('19. Запись обновлена по ROWID');
        ROLLBACK;
    END IF;

    -- 20
    DBMS_OUTPUT.PUT_LINE('20. Преподаватели');
    FOR rec IN (SELECT TEACHER_NAME FROM TEACHER) LOOP
        v_counter := v_counter + 1;
        DBMS_OUTPUT.PUT_LINE(rec.TEACHER_NAME);
        IF MOD(v_counter, 3) = 0 THEN
            DBMS_OUTPUT.PUT_LINE('---');
        END IF;
    END LOOP;
END;