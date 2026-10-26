--10
CREATE TABLE test_table(
	id NUMBER PRIMARY KEY,
	name varchar2(100)
);

CREATE VIEW test_view AS
SELECT id, name
FROM test_table
WHERE id>0;


