ALTER SESSION SET CONTAINER = CDB$ROOT;

-- 2. Включаем автоуправление файлами (OMF), чтобы Oracle сам создал папку под новую БД в контейнере
ALTER SYSTEM SET db_create_file_dest = '/opt/oracle/oradata' SCOPE=BOTH;

-- 3. Создаем нашу собственную подключаемую базу данных CLINIC_PDB
CREATE PLUGGABLE DATABASE CLINIC_PDB
    ADMIN USER pdb_admin IDENTIFIED BY "KostusLi";

-- 4. Открываем созданную БД для чтения и записи
ALTER PLUGGABLE DATABASE CLINIC_PDB OPEN;

-- 5. Сохраняем состояние, чтобы CLINIC_PDB сама автоматически открывалась при перезапуске Docker
ALTER PLUGGABLE DATABASE CLINIC_PDB SAVE STATE;