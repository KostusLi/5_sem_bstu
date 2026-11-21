CREATE OR REPLACE PACKAGE PKG_JSON_IO AS
    -- 1. Экспорт справочника услуг (прайс-листа) в JSON-файл на сервере (DIR_CLINIC_JSON)
    PROCEDURE export_pricelist_to_file(
        p_filename IN VARCHAR2 DEFAULT 'pricelist_export.json'
    );

    -- 2. Экспорт медицинских карт и анализов пациентов в JSON-файл
    PROCEDURE export_medcards_to_file(
        p_patient_id IN NUMBER DEFAULT NULL,
        p_filename   IN VARCHAR2 DEFAULT 'medical_cards_export.json'
    );

    -- 3. Импорт услуг прайс-листа из JSON-строки (CLOB) через JSON_TABLE
    PROCEDURE import_pricelist_from_clob(
        p_json_data      IN  CLOB,
        p_processed_rows OUT NUMBER
    );

    -- 4. Импорт услуг прайс-листа из JSON-файла на сервере (DIR_CLINIC_JSON)
    PROCEDURE import_pricelist_from_file(
        p_filename       IN  VARCHAR2 DEFAULT 'pricelist_export.json',
        p_processed_rows OUT NUMBER
    );
END PKG_JSON_IO;


CREATE OR REPLACE PACKAGE BODY PKG_JSON_IO AS

    PROCEDURE write_clob_to_file(p_filename IN VARCHAR2, p_content IN CLOB) IS
        v_file   UTL_FILE.FILE_TYPE;
        v_pos    INTEGER := 1;
        v_amount BINARY_INTEGER := 4000;
        v_len    INTEGER := DBMS_LOB.GETLENGTH(p_content);
        v_buffer VARCHAR2(16000);
    BEGIN
        v_file := UTL_FILE.FOPEN('DIR_CLINIC_JSON', p_filename, 'wb', 32767);
        WHILE v_pos <= v_len LOOP
            DBMS_LOB.READ(p_content, v_amount, v_pos, v_buffer);
            UTL_FILE.PUT_RAW(
                v_file,
                UTL_I18N.STRING_TO_RAW(v_buffer, 'AL32UTF8'),
                TRUE
            );
            v_pos := v_pos + v_amount;
        END LOOP;
        UTL_FILE.FCLOSE(v_file);
    EXCEPTION
        WHEN OTHERS THEN
            IF UTL_FILE.IS_OPEN(v_file) THEN
                UTL_FILE.FCLOSE(v_file);
            END IF;
            RAISE;
    END write_clob_to_file;

    PROCEDURE export_pricelist_to_file(
        p_filename IN VARCHAR2 DEFAULT 'pricelist_export.json'
    ) IS
        v_json_clob CLOB;
    BEGIN
        SELECT JSON_SERIALIZE(
            JSON_OBJECT(
                'export_date' VALUE TO_CHAR(SYSDATE, 'YYYY-MM-DD HH24:MI:SS'),
                'clinic'      VALUE 'Medical Clinic DB',
                'services'    VALUE JSON_ARRAYAGG(
                    JSON_OBJECT(
                        'service_id'     VALUE s.service_id,
                        'specialty_name' VALUE sp.specialty_name,
                        'service_name'   VALUE s.service_name,
                        'price'          VALUE s.price
                    ) ORDER BY sp.specialty_name, s.service_name
                    RETURNING CLOB
                )
                RETURNING CLOB
            ) RETURNING CLOB PRETTY
        )
        INTO v_json_clob
        FROM SERVICES s
        JOIN SPECIALTIES sp ON s.specialty_id = sp.specialty_id;

        write_clob_to_file(p_filename, v_json_clob);
    END export_pricelist_to_file;

    PROCEDURE export_medcards_to_file(
        p_patient_id IN NUMBER DEFAULT NULL,
        p_filename   IN VARCHAR2 DEFAULT 'medical_cards_export.json'
    ) IS
        v_json_clob CLOB;
    BEGIN
        SELECT JSON_SERIALIZE(
            JSON_OBJECT(
                'export_date' VALUE TO_CHAR(SYSDATE, 'YYYY-MM-DD HH24:MI:SS'),
                'records'     VALUE JSON_ARRAYAGG(
                    JSON_OBJECT(
                        'record_id'        VALUE mr.record_id,
                        'visit_date'       VALUE TO_CHAR(a.slot_start, 'YYYY-MM-DD HH24:MI'),
                        'patient_name'     VALUE p.full_name,
                        'policy_number'    VALUE p.policy_number,
                        'doctor_name'      VALUE u.full_name,
                        'specialty'        VALUE sp.specialty_name,
                        'complaints'       VALUE mr.complaints,
                        'diagnosis'        VALUE mr.diagnosis,
                        'prescriptions'    VALUE mr.prescriptions,
                        'analyses_results' VALUE mr.analyses_results
                        RETURNING CLOB
                    )
                    RETURNING CLOB
                )
                RETURNING CLOB
            ) RETURNING CLOB PRETTY
        )
        INTO v_json_clob
        FROM (
            SELECT * FROM MEDICAL_RECORDS 
            ORDER BY record_id DESC 
            FETCH FIRST 200 ROWS ONLY
        ) mr
        JOIN APPOINTMENTS a ON mr.appointment_id = a.appointment_id
        JOIN PATIENTS p     ON a.patient_id = p.patient_id
        JOIN DOCTORS d      ON a.doctor_id = d.doctor_id
        JOIN USERS u        ON d.user_id = u.user_id
        JOIN SPECIALTIES sp ON d.specialty_id = sp.specialty_id
        WHERE (p_patient_id IS NULL OR p.patient_id = p_patient_id);

        write_clob_to_file(p_filename, v_json_clob);
    END export_medcards_to_file;

    PROCEDURE import_pricelist_from_clob(
        p_json_data      IN  CLOB,
        p_processed_rows OUT NUMBER
    ) IS
        v_count NUMBER := 0;
    BEGIN
        MERGE INTO SPECIALTIES sp
        USING (
            SELECT DISTINCT TRIM(jt.specialty_name) AS specialty_name
            FROM JSON_TABLE(p_json_data, '$.services[*]'
                COLUMNS (
                    specialty_name VARCHAR2(100) PATH '$.specialty_name'
                )
            ) jt
            WHERE jt.specialty_name IS NOT NULL
        ) src
        ON (UPPER(sp.specialty_name) = UPPER(src.specialty_name))
        WHEN NOT MATCHED THEN
            INSERT (specialty_name) VALUES (src.specialty_name);

        MERGE INTO SERVICES s
        USING (
            SELECT 
                sp.specialty_id,
                TRIM(jt.service_name) AS service_name,
                jt.price
            FROM JSON_TABLE(p_json_data, '$.services[*]'
                COLUMNS (
                    specialty_name VARCHAR2(100) PATH '$.specialty_name',
                    service_name   VARCHAR2(200) PATH '$.service_name',
                    price          NUMBER(10, 2) PATH '$.price'
                )
            ) jt
            JOIN SPECIALTIES sp ON UPPER(sp.specialty_name) = UPPER(TRIM(jt.specialty_name))
            WHERE jt.service_name IS NOT NULL AND jt.price >= 0
        ) src
        ON (s.specialty_id = src.specialty_id AND UPPER(s.service_name) = UPPER(src.service_name))
        WHEN MATCHED THEN
            UPDATE SET s.price = src.price
        WHEN NOT MATCHED THEN
            INSERT (specialty_id, service_name, price)
            VALUES (src.specialty_id, src.service_name, src.price);

        v_count := SQL%ROWCOUNT;
        p_processed_rows := v_count;
        COMMIT;
    END import_pricelist_from_clob;

    PROCEDURE import_pricelist_from_file(
        p_filename       IN  VARCHAR2 DEFAULT 'pricelist_export.json',
        p_processed_rows OUT NUMBER
    ) IS
        v_bfile       BFILE;
        v_clob        CLOB;
        v_dest_offset INTEGER := 1;
        v_src_offset  INTEGER := 1;
        v_lang_ctx    INTEGER := DBMS_LOB.DEFAULT_LANG_CTX;
        v_warning     INTEGER;
    BEGIN
        v_bfile := BFILENAME('DIR_CLINIC_JSON', p_filename);
        DBMS_LOB.FILEOPEN(v_bfile, DBMS_LOB.FILE_READONLY);
        DBMS_LOB.CREATETEMPORARY(v_clob, TRUE);

        DBMS_LOB.LOADCLOBFROMFILE(
            dest_lob     => v_clob,
            src_bfile    => v_bfile,
            amount       => DBMS_LOB.LOBMAXSIZE,
            dest_offset  => v_dest_offset,
            src_offset   => v_src_offset,
            bfile_csid   => NLS_CHARSET_ID('AL32UTF8'),
            lang_context => v_lang_ctx,
            warning      => v_warning
        );

        DBMS_LOB.FILECLOSE(v_bfile);

        import_pricelist_from_clob(v_clob, p_processed_rows);

        DBMS_LOB.FREETEMPORARY(v_clob);
    EXCEPTION
        WHEN OTHERS THEN
            IF DBMS_LOB.FILEISOPEN(v_bfile) = 1 THEN
                DBMS_LOB.FILECLOSE(v_bfile);
            END IF;
            RAISE;
    END import_pricelist_from_file;

END PKG_JSON_IO;