SELECT object_name, object_type, status 
FROM user_objects 
WHERE object_type IN ('PACKAGE', 'PACKAGE BODY')
ORDER BY object_name, object_type;