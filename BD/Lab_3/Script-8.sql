--1
SELECT name, open_mode
FROM v$pdbs;

--2
SELECT instance_name, host_name, version, status
FROM v$instance;

--3
SELECT comp_name, version, status
FROM dba_registry;

--4
SELECT name
FROM v$datafile
WHERE name LIKE '%pdbseed%'

CREATE pluggable DATABASE HKV_PDB
admin USER admin_hkv IDENTIFIED BY "12345"
file_name_convert = ('/pdbseed/', '/HKV_PDB/');

ALTER SESSION SET container = CDB$ROOT;

SELECT sys_context('USERENV', 'CON_NAME') FROM dual;

--5
SELECT name, open_mode
FROM v$pdbs

ALTER pluggable DATABASE HKV_PDB OPEN

--6
ALTER SESSION SET container = HKV_PDB

CREATE TABLESPACE hkv_data
DATAFILE 'hkv_data_01.dbf' SIZE 50M autoextend ON NEXT 10M maxsize 100M

CREATE TEMPORARY TABLESPACE hkv_temp
TEMPFILE 'hkv_temp_01.dbf' SIZE 20M autoextend ON NEXT 5M

CREATE profile hkv_profile LIMIT
password_life_time 180
sessions_per_user 5
failed_login_attempts 3

CREATE ROLE hkv_role;
GRANT CREATE SESSION, CREATE TABLE, CREATE VIEW TO hkv_role;

CREATE USER u1_hkv_pdb IDENTIFIED BY "12345"
DEFAULT TABLESPACE hkv_data
TEMPORARY TABLESPACE hkv_temp
profile hkv_profile;

ALTER USER u1_hkv_pdb quota unlimited ON hkv_data;

GRANT hkv_role TO u1_hkv_pdb;

ALTER SESSION SET container = HKV_PDB;

GRANT CREATE SESSION TO u1_hkv_pdb


--8
ALTER SESSION SET CONTAINER = HKV_PDB;

SELECT t.tablespace_name, t.contents, d.file_name, d.bytes/1024/1024 as size_mb
FROM dba_tablespaces t
LEFT JOIN dba_data_files d ON t.tablespace_name = d.tablespace_name
ORDER BY t.tablespace_name;

SELECT tablespace_name, file_name, bytes/1024/1024 as size_mb 
FROM dba_temp_files;

SELECT username, profile, account_status, created 
FROM dba_users 
WHERE username NOT IN ('SYS', 'SYSTEM')
ORDER BY username;

SELECT profile, resource_name, limit 
FROM dba_profiles 
WHERE profile = 'HKV_PROFILE';

SELECT role, privilege 
FROM role_sys_privs 
WHERE role = 'hkv_role';

SELECT grantee, granted_role
FROM dba_role_privs
WHERE grantee = 'u1_hrv_pdb'

--9
ALTER SESSION SET CONTAINER = CDB$ROOT;

CREATE USER C##HKV IDENTIFIED BY "12345" CONTAINER = ALL;

GRANT CREATE SESSION TO C##HKV container = ALL;

SELECT sys_context('USERENV', 'CON_NAME') as CURRENT_CONTAINER FROM dual;
