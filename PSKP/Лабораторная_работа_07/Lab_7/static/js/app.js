fetch('/data/info.json')
  .then(r => r.json())
  .then(obj => {
    document.getElementById('json-out').textContent =
      'JSON:\n' + JSON.stringify(obj, null, 2);
  })
  .catch(e => {
    document.getElementById('json-out').textContent = 'Ошибка JSON: ' + e;
  });

fetch('/data/note.xml')
  .then(r => r.text())
  .then(txt => {
    document.getElementById('xml-out').textContent = 'XML:\n' + txt;
  })
  .catch(e => {
    document.getElementById('xml-out').textContent = 'Ошибка XML: ' + e;
  });