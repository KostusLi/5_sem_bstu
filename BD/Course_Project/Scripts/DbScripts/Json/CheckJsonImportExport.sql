DECLARE
    v_imported_rows NUMBER;
BEGIN
    PKG_JSON_IO.export_pricelist_to_file('pricelist_export.json');
    PKG_JSON_IO.export_medcards_to_file(NULL, 'medical_cards_export.json');
    PKG_JSON_IO.import_pricelist_from_file('pricelist_export.json', v_imported_rows);
    DBMS_OUTPUT.PUT_LINE('Успешно экспортировано и импортировано услуг из JSON: ' || v_imported_rows);
END;