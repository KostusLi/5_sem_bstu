-- 1. Смотрим содержимое файла /tmp/pricelist_export.json
SELECT TO_CLOB(BFILENAME('DIR_CLINIC_JSON', 'pricelist_export.json'), NLS_CHARSET_ID('AL32UTF8')) AS json_content 
FROM DUAL;

-- 2. Смотрим содержимое файла /tmp/medical_cards_export.json (с результатами анализов)
SELECT TO_CLOB(BFILENAME('DIR_CLINIC_JSON', 'medical_cards_export.json'), NLS_CHARSET_ID('AL32UTF8')) AS json_content 
FROM DUAL;