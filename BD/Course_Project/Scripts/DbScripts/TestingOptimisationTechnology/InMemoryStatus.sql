-- Количество строк во всех таблицах (APPOINTMENTS = 374 400)
SELECT 'ROLES' AS table_name, COUNT(*) AS row_count FROM ROLES
UNION ALL SELECT 'SPECIALTIES', COUNT(*) FROM SPECIALTIES
UNION ALL SELECT 'SERVICES', COUNT(*) FROM SERVICES
UNION ALL SELECT 'USERS', COUNT(*) FROM USERS
UNION ALL SELECT 'DOCTORS', COUNT(*) FROM DOCTORS
UNION ALL SELECT 'PATIENTS', COUNT(*) FROM PATIENTS
UNION ALL SELECT 'APPOINTMENTS', COUNT(*) FROM APPOINTMENTS
UNION ALL SELECT 'APPOINTMENT_SERVICES', COUNT(*) FROM APPOINTMENT_SERVICES
UNION ALL SELECT 'PAYMENTS', COUNT(*) FROM PAYMENTS
UNION ALL SELECT 'MEDICAL_RECORDS', COUNT(*) FROM MEDICAL_RECORDS;

-- Выделенная память под SGA, PGA и область In-Memory (304 МБ)
SELECT name, ROUND(value / 1024 / 1024, 2) AS size_mb
FROM V$PARAMETER
WHERE name IN ('sga_max_size', 'inmemory_size', 'pga_aggregate_target');

-- Загруженные в In-Memory таблицы, их размер в RAM и коэффициент сжатия
SELECT 
    segment_name,
    ROUND(bytes / 1024 / 1024, 2)              AS disk_size_mb,
    ROUND(inmemory_size / 1024 / 1024, 2)      AS inmemory_size_mb,
    ROUND(bytes / NULLIF(inmemory_size, 0), 2) AS compression_ratio,
    populate_status,
    inmemory_compression
FROM V$IM_SEGMENTS
ORDER BY inmemory_size DESC;