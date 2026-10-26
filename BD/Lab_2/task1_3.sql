-- 1
CREATE TABLESPACE ts_hkv
DATAFILE 'TS_HKV.dbf' SIZE 7M
autoextend ON NEXT 5M maxsize 20M

-- 2
CREATE TEMPORARY TABLESPACE ts_hkv_temp
TEMPFILE 'TS_HKV_TEMP.dbf' SIZE 5M
autoextend ON NEXT 3M maxsize 30M


-- 3
SELECT 
    t.tablespace_name AS "Имя пространства",
    t.contents AS "Тип",
    f.file_name AS "Путь к файлу",
    f.bytes / 1024 / 1024 AS "Текущий размер (МБ)"
FROM dba_tablespaces t
JOIN dba_data_files f ON t.tablespace_name = f.tablespace_name
UNION ALL
SELECT 
    t.tablespace_name,
    t.contents,
    f.file_name,
    f.bytes / 1024 / 1024
FROM dba_tablespaces t
JOIN dba_temp_files f ON t.tablespace_name = f.tablespace_name
ORDER BY "Имя пространства";


