-- 4
CREATE ROLE RL_HKVCORE;

GRANT 
	CREATE SESSION,
	CREATE TABLE,
	CREATE VIEW,
	CREATE PROCEDURE
TO RL_HKVCORE

-- 5
SELECT ROLE, password_required, authentication_type
FROM dba_roles
WHERE ROLE='RL_HKVCORE';

SELECT grantee, privilege, admin_option
FROM dba_sys_privs
WHERE grantee = 'RL_HKVCORE'
ORDER BY privilege;

SELECT file_name FROM dba_data_files WHERE tablespace_name = 'TS_HKV';