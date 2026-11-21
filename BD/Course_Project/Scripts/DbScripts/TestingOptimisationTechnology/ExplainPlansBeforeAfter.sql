-- 1А. Поиск приёмов пациента БЕЗ индекса и БЕЗ In-Memory (TABLE ACCESS FULL, Cost = 547)
EXPLAIN PLAN FOR
SELECT /*+ FULL(a) NO_INMEMORY(a) */
       appointment_id, doctor_id, slot_start, cabinet_number, status
FROM APPOINTMENTS a
WHERE patient_id = 125 AND status = 'BOOKED';

SELECT * FROM TABLE(DBMS_XPLAN.DISPLAY(FORMAT => 'BASIC +COST +BYTES'));

-- 1Б. Поиск приёмов пациента ЧЕРЕЗ созданный B-Tree индекс (INDEX RANGE SCAN, Cost = 10)
EXPLAIN PLAN FOR
SELECT /*+ INDEX(a IDX_APPOINTMENTS_PATIENT) */
       appointment_id, doctor_id, slot_start, cabinet_number, status
FROM APPOINTMENTS a
WHERE patient_id = 125 AND status = 'BOOKED';

SELECT * FROM TABLE(DBMS_XPLAN.DISPLAY(FORMAT => 'BASIC +COST +BYTES'));

-- 2А. Финансовый отчёт за 3 года БЕЗ In-Memory (TABLE ACCESS FULL, Cost = 2614)
EXPLAIN PLAN FOR
SELECT /*+ NO_INMEMORY(pay) NO_INMEMORY(a) NO_INMEMORY(d) NO_INMEMORY(sp) */
       sp.specialty_name, TO_CHAR(a.slot_start, 'YYYY-MM') AS report_month,
       COUNT(pay.payment_id), SUM(pay.amount), SUM(pay.final_amount)
FROM PAYMENTS pay
JOIN APPOINTMENTS a ON pay.appointment_id = a.appointment_id
JOIN DOCTORS d      ON a.doctor_id = d.doctor_id
JOIN SPECIALTIES sp ON d.specialty_id = sp.specialty_id
WHERE pay.status = 'PAID'
  AND a.slot_start >= TIMESTAMP '2024-01-01 00:00:00'
  AND a.slot_start <  TIMESTAMP '2027-01-01 00:00:00'
GROUP BY sp.specialty_name, TO_CHAR(a.slot_start, 'YYYY-MM');

SELECT * FROM TABLE(DBMS_XPLAN.DISPLAY(FORMAT => 'BASIC +COST +BYTES'));

-- 2Б. Финансовый отчёт за 3 года С In-Memory (TABLE ACCESS INMEMORY FULL, Cost сканирования 26 и 13, общий = 1822)
EXPLAIN PLAN FOR
SELECT /*+ VECTOR_TRANSFORM */
       sp.specialty_name, yr || '-' || LPAD(mn, 2, '0') AS report_month,
       cnt, gross_amt, net_rev
FROM (
    SELECT d.specialty_id,
           EXTRACT(YEAR FROM a.slot_start)  AS yr,
           EXTRACT(MONTH FROM a.slot_start) AS mn,
           COUNT(pay.payment_id)            AS cnt,
           SUM(pay.amount)                  AS gross_amt,
           SUM(pay.final_amount)            AS net_rev
    FROM PAYMENTS pay
    JOIN APPOINTMENTS a ON pay.appointment_id = a.appointment_id
    JOIN DOCTORS d      ON a.doctor_id = d.doctor_id
    WHERE pay.status = 'PAID'
      AND a.slot_start >= TIMESTAMP '2024-01-01 00:00:00'
      AND a.slot_start <  TIMESTAMP '2027-01-01 00:00:00'
    GROUP BY d.specialty_id, EXTRACT(YEAR FROM a.slot_start), EXTRACT(MONTH FROM a.slot_start)
) agg
JOIN SPECIALTIES sp ON agg.specialty_id = sp.specialty_id;

SELECT * FROM TABLE(DBMS_XPLAN.DISPLAY(FORMAT => 'BASIC +COST +BYTES'));

-- 3А. Загруженность врачей (374 400 строк) БЕЗ In-Memory (TABLE ACCESS FULL, Cost = 548)
EXPLAIN PLAN FOR
SELECT /*+ NO_INMEMORY(a) NO_INMEMORY(d) */
       d.doctor_id, COUNT(a.appointment_id) AS total_slots,
       SUM(CASE WHEN a.status IN ('BOOKED', 'COMPLETED') THEN 1 ELSE 0 END) AS busy_slots
FROM DOCTORS d
JOIN APPOINTMENTS a ON d.doctor_id = a.doctor_id
GROUP BY d.doctor_id;

SELECT * FROM TABLE(DBMS_XPLAN.DISPLAY(FORMAT => 'BASIC +COST +BYTES'));

-- 3Б. Загруженность врачей (374 400 строк) С In-Memory (TABLE ACCESS INMEMORY FULL, Cost = 26)
EXPLAIN PLAN FOR
SELECT d.doctor_id, COUNT(a.appointment_id) AS total_slots,
       SUM(CASE WHEN a.status IN ('BOOKED', 'COMPLETED') THEN 1 ELSE 0 END) AS busy_slots
FROM DOCTORS d
JOIN APPOINTMENTS a ON d.doctor_id = a.doctor_id
GROUP BY d.doctor_id;

SELECT * FROM TABLE(DBMS_XPLAN.DISPLAY(FORMAT => 'BASIC +COST +BYTES'));