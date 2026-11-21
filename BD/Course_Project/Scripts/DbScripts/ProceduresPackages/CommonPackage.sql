CREATE OR REPLACE PACKAGE PKG_COMMON AS
    -- Функция хэширования пароля алгоритмом SHA-256
    FUNCTION fn_hash_password(p_password IN VARCHAR2) RETURN VARCHAR2;

    -- 1. Авторизация и аутентификация в системе
    PROCEDURE authenticate_user(
        p_login     IN  VARCHAR2,
        p_password  IN  VARCHAR2,
        p_user_id   OUT NUMBER,
        p_role_name OUT VARCHAR2,
        p_full_name OUT VARCHAR2,
        p_doctor_id OUT NUMBER
    );

    -- 2. Смена пароля собственной учётной записи
    PROCEDURE change_password(
        p_user_id      IN NUMBER,
        p_old_password IN VARCHAR2,
        p_new_password IN VARCHAR2
    );

    -- 3. Просмотр актуального справочника медицинских услуг (прайс-листа)
    PROCEDURE get_pricelist(
        p_specialty_id IN  NUMBER DEFAULT NULL,
        p_cursor       OUT SYS_REFCURSOR
    );

    -- 4. Просмотр общего расписания работы врачей клиники
    PROCEDURE get_general_schedule(
        p_date_from    IN  DATE,
        p_date_to      IN  DATE,
        p_specialty_id IN  NUMBER DEFAULT NULL,
        p_doctor_id    IN  NUMBER DEFAULT NULL,
        p_cursor       OUT SYS_REFCURSOR
    );
END PKG_COMMON;



CREATE OR REPLACE PACKAGE BODY PKG_COMMON AS

    FUNCTION fn_hash_password(p_password IN VARCHAR2) RETURN VARCHAR2 IS
    BEGIN
        IF p_password IS NULL OR LENGTH(TRIM(p_password)) < 4 THEN
            RAISE_APPLICATION_ERROR(-20010, 'Пароль должен содержать не менее 4 символов.');
        END IF;
        RETURN RAWTOHEX(
            SYS.DBMS_CRYPTO.HASH(
                UTL_I18N.STRING_TO_RAW(p_password, 'AL32UTF8'),
                SYS.DBMS_CRYPTO.HASH_SH256
            )
        );
    END fn_hash_password;

    PROCEDURE authenticate_user(
        p_login     IN  VARCHAR2,
        p_password  IN  VARCHAR2,
        p_user_id   OUT NUMBER,
        p_role_name OUT VARCHAR2,
        p_full_name OUT VARCHAR2,
        p_doctor_id OUT NUMBER
    ) IS
        v_hash      USERS.password_hash%TYPE;
        v_is_active USERS.is_active%TYPE;
    BEGIN
        v_hash := fn_hash_password(p_password);

        SELECT u.user_id, r.role_name, u.full_name, u.is_active, d.doctor_id
        INTO p_user_id, p_role_name, p_full_name, v_is_active, p_doctor_id
        FROM USERS u
        JOIN ROLES r ON u.role_id = r.role_id
        LEFT JOIN DOCTORS d ON u.user_id = d.user_id
        WHERE LOWER(u.login) = LOWER(TRIM(p_login))
          AND u.password_hash = v_hash;

        IF v_is_active = 0 THEN
            RAISE_APPLICATION_ERROR(-20011, 'Учётная запись заблокирована администратором.');
        END IF;
    EXCEPTION
        WHEN NO_DATA_FOUND THEN
            RAISE_APPLICATION_ERROR(-20012, 'Неверный логин или пароль.');
    END authenticate_user;

    PROCEDURE change_password(
        p_user_id      IN NUMBER,
        p_old_password IN VARCHAR2,
        p_new_password IN VARCHAR2
    ) IS
        v_current_hash USERS.password_hash%TYPE;
    BEGIN
        SELECT password_hash INTO v_current_hash
        FROM USERS
        WHERE user_id = p_user_id;

        IF v_current_hash <> fn_hash_password(p_old_password) THEN
            RAISE_APPLICATION_ERROR(-20013, 'Старый пароль указан неверно.');
        END IF;

        UPDATE USERS
        SET password_hash = fn_hash_password(p_new_password)
        WHERE user_id = p_user_id;

        COMMIT;
    EXCEPTION
        WHEN NO_DATA_FOUND THEN
            RAISE_APPLICATION_ERROR(-20014, 'Пользователь не найден.');
    END change_password;

    PROCEDURE get_pricelist(
        p_specialty_id IN  NUMBER DEFAULT NULL,
        p_cursor       OUT SYS_REFCURSOR
    ) IS
    BEGIN
        OPEN p_cursor FOR
            SELECT service_id, specialty_id, specialty_name, service_name, price
            FROM V_PRICELIST
            WHERE (p_specialty_id IS NULL OR specialty_id = p_specialty_id)
            ORDER BY specialty_name, service_name;
    END get_pricelist;

    PROCEDURE get_general_schedule(
        p_date_from    IN  DATE,
        p_date_to      IN  DATE,
        p_specialty_id IN  NUMBER DEFAULT NULL,
        p_doctor_id    IN  NUMBER DEFAULT NULL,
        p_cursor       OUT SYS_REFCURSOR
    ) IS
    BEGIN
        OPEN p_cursor FOR
            SELECT appointment_id, slot_start, cabinet_number, status,
                   doctor_id, doctor_name, specialty_id, specialty_name
            FROM V_SCHEDULE_DETAILS
            WHERE slot_start >= CAST(TRUNC(p_date_from) AS TIMESTAMP)
              AND slot_start <  CAST(TRUNC(p_date_to) + 1 AS TIMESTAMP)
              AND (p_specialty_id IS NULL OR specialty_id = p_specialty_id)
              AND (p_doctor_id IS NULL OR doctor_id = p_doctor_id)
              AND doctor_active = 1
            ORDER BY slot_start, specialty_name, doctor_name;
    END get_general_schedule;

END PKG_COMMON;