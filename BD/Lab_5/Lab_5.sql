SELECT sum(value)/1024/1024 AS "Total_SGA_MB" FROM v$sga;

SELECT pool, sum(bytes)/1024/1024 AS "Pool_Size_MB" FROM v$sgastat GROUP BY pool;

SELECT component, granule_size FROM v$sga_dynamic_components;

SELECT name, value/1024/1024 AS "Size_MB" 
FROM v$parameter 
WHERE name IN ('sga_max_size', 'sga_target');

SELECT component, current_size/1024/1024 AS "Size_MB" 
FROM v$sga_dynamic_components 
WHERE component LIKE '%buffer cache%';

CREATE TABLE table_keep (id NUMBER) STORAGE (BUFFER_POOL KEEP);
SELECT segment_name, buffer_pool FROM user_segments WHERE segment_name = 'TABLE_KEEP';

CREATE TABLE table_default (id NUMBER) CACHE;
SELECT segment_name, buffer_pool FROM user_segments WHERE segment_name = 'TABLE_DEFAULT';


SELECT name, value FROM v$sga WHERE name = 'Redo Buffers';

SELECT pool, name, bytes FROM v$sgastat 
WHERE pool = 'large pool' AND name = 'free memory';

SELECT sid, serial#, server FROM v$session;

SELECT pname, program FROM v$process WHERE background = 1;

SELECT program FROM v$process WHERE background IS NULL;

SELECT count(*) FROM v$process WHERE pname LIKE 'DBW%';

SELECT name FROM v$services;

SELECT name, value FROM v$parameter WHERE name = 'dispatchers';


