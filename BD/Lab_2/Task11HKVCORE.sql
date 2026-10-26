CREATE TABLE HKV_T1 (
    id NUMBER,
    text_data VARCHAR2(50)
) TABLESPACE HKV_QDATA;

INSERT INTO HKV_T1 (id, text_data) VALUES (1, 'Строка номер один');
INSERT INTO HKV_T1 (id, text_data) VALUES (2, 'Строка номер два');
INSERT INTO HKV_T1 (id, text_data) VALUES (3, 'Строка номер три');
