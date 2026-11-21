-- Таблица времени выполнения 4 тестовых запросов (в мс)
SELECT 1 AS test_no, '1. Поиск приёмов пациента (patient_id)' AS query_name, FN_BENCHMARK_MS(1) AS time_ms FROM DUAL
UNION ALL
SELECT 2, '2. Финансовый отчёт по специальностям (3 года)', FN_BENCHMARK_MS(2) FROM DUAL
UNION ALL
SELECT 3, '3. Анализ загруженности врачей (374 400 строк)', FN_BENCHMARK_MS(3) FROM DUAL
UNION ALL
SELECT 4, '4. Прогнозирование популярности услуг (тренды)', FN_BENCHMARK_MS(4) FROM DUAL;