CREATE profile PF_HKVCORE LIMIT
	password_life_time 180
	sessions_per_user 3
	failed_login_attempts 7
	password_lock_time 1
	password_reuse_time 10
	password_grace_time DEFAULT
	connect_time 180
	idle_time 30;

SELECT DISTINCT profile
FROM dba_profiles
ORDER BY profile;

SELECT resource_name AS "Имя параметра", resource_type AS "Тип", LIMIT AS "Значение"
FROM dba_profiles
WHERE profile = 'PF_HKVCORE';

SELECT resource_name AS "Имя параметра", resource_type AS "Тип", LIMIT AS "Значение"
FROM dba_profiles
WHERE profile='DEFAULT'