--1
SELECT tablespace_name, file_name, bytes, autoextensible
FROM dba_data_files
ORDER BY tablespace_name;

SELECT tablespace_name, file_name, bytes, autoextensible
FROM dba_temp_files
ORDER BY tablespace_name;

--8
DROP TABLESPACE HKV_QDATA INCLUDING CONTENTS AND DATAFILES CASCADE CONSTRAINTS;

--9
SELECT group#, members, bytes, status, sequence#
FROM v$log;

--10
SELECT group#, member, status
FROM v$logfile
ORDER BY group#;

--11
SELECT SYSDATE FROM dual;

SELECT group#, status, sequence# FROM v$log;

ALTER SYSTEM SWITCH LOGFILE;

SELECT group#, status, sequence# FROM v$log;

--12
ALTER DATABASE ADD LOGFILE GROUP 4 (
  'HKV_redo04a.log',
  'HKV_redo04b.log',
  'HKV_redo04c.log'
) SIZE 50M;

SELECT group#, members, status, sequence# FROM v$log;
SELECT group#, member FROM v$logfile WHERE group# = 4;

ALTER SYSTEM SWITCH LOGFILE;
SELECT group#, status, sequence#, first_change# FROM v$log;

--13
SELECT group#, status FROM v$log WHERE group# = 4;
ALTER SYSTEM CHECKPOINT;

ALTER DATABASE DROP LOGFILE GROUP 4;

--14-15
SELECT log_mode FROM v$database;

SELECT max(sequence#) FROM v$archived_log;

ALTER SESSION SET CONTAINER = CDB$ROOT;

--17
ALTER SYSTEM ARCHIVE LOG CURRENT;

SELECT sequence#, name AS archive_location, first_change# AS start_scn, next_change# AS end_scn
FROM v$archived_log
ORDER BY sequence# DESC
FETCH FIRST 5 ROWS ONLY; 

SELECT group#, sequence#, status, first_change# AS start_scn
FROM v$log;

--19
SELECT name, status FROM v$controlfile;

--20
SELECT type, record_size, records_total, records_used 
FROM v$controlfile_record_section;

--21
SELECT value AS spfile_location 
FROM v$parameter 
WHERE name = 'spfile';

--22
CREATE PFILE='HKV_PFILE.ORA' FROM SPFILE;

--23
SELECT * FROM v$pwfile_users;

--24
SELECT name, value 
FROM v$diag_info;


--25
SELECT TO_CHAR(originating_timestamp, 'DD.MM.YYYY HH24:MI:SS') AS log_time, 
       message_text
FROM v$diag_alert_ext
WHERE message_text LIKE '%advanced to log sequence%' OR message_text LIKE '%SWITCH LOGFILE%'
ORDER BY originating_timestamp DESC
FETCH FIRST 20 ROWS ONLY;
