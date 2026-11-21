const http = require('http');

const x = 10;
const y = 5;

const options = {
    hostname: 'localhost',
    port: 3000,
    path: `/parameter?x=${x}&y=${y}`,
    method: 'GET'
};

const req = http.request(options, (res) => {
    console.log('=== Задание 02 ===');
    console.log(`Статус: ${res.statusCode}`);
    
    let data = '';
    res.on('data', chunk => data += chunk);
    res.on('end', () => {
        console.log(`Тело ответа:\n${data}`);
    });
});

req.on('error', (e) => console.error(`Ошибка: ${e.message}`));
req.end();