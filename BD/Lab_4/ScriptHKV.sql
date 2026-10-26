--2
CREATE TABLE HKV_T1 (
    id     NUMBER PRIMARY KEY,
    val    VARCHAR2(50)
) TABLESPACE HKV_QDATA;

INSERT INTO HKV_T1 VALUES (1, 'row1');
INSERT INTO HKV_T1 VALUES (2, 'row2');
INSERT INTO HKV_T1 VALUES (3, 'row3');
COMMIT;

--3
SELECT segment_name, segment_type, bytes, blocks
FROM user_segments
WHERE tablespace_name = 'HKV_QDATA';


--4 
DROP TABLE HKV_T1;
SELECT object_name, original_name, type, droptime, can_undrop FROM user_recyclebin; 

--5
FLASHBACK TABLE HKV_T1 TO BEFORE DROP;



--6
BEGIN
  FOR i IN 4..10003 LOOP
    INSERT INTO HKV_T1 VALUES (i, 'row' || i);
  END LOOP;
  COMMIT;
END;

--7
SELECT segment_name, 
       COUNT(extent_id) AS extents_count, 
       SUM(blocks) AS total_blocks, 
       SUM(bytes) AS total_bytes
FROM user_extents
WHERE segment_name = 'HKV_T1'
GROUP BY segment_name;

SELECT extent_id, bytes, blocks
FROM user_extents
WHERE segment_name = 'HKV_T1'
ORDER BY extent_id;

