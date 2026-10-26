--7
CREATE TABLE HKV_table (
    id NUMBER GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    band_name VARCHAR2(100) NOT NULL,
    genre VARCHAR2(50),
    formation_year NUMBER(4)
);

INSERT INTO HKV_table (band_name, genre, formation_year) VALUES ('Green Day', 'Punk Rock', 1987);
INSERT INTO HKV_table (band_name, genre, formation_year) VALUES ('The Killers', 'Alt Rock', 2001);
INSERT INTO HKV_table (band_name, genre, formation_year) VALUES ('Shinedown', 'Hard Rock', 2001);

SELECT * FROM HKV_table;