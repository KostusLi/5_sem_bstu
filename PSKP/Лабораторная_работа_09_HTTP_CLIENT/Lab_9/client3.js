const http = require('http');
const querystring = require('querystring');

const postData = querystring.stringify({
    x: 10,
    y: 5,
    s: 'Hello from client 03'
});

const options = {
    hostname: 'localhost',
    port: 3000,
    path: '/formparameter', // Используем эндпоинт из лабы 8, который парсит form-data
    method: 'POST',
    headers: {
        'Content-Type': 'application/x-www-form-urlencoded',
        'Content-Length': Buffer.byteLength(postData)
    }
};

const req = http.request(options, (res) => {
    console.log('=== Задание 03 ===');
    console.log(`Статус: ${res.statusCode}`);
    
    let data = '';
    res.on('data', chunk => data += chunk);
    res.on('end', () => {
        console.log(`Тело ответа:\n${data}`);
    });
});

req.on('error', (e) => console.error(`Ошибка: ${e.message}`));
req.write(postData);
req.end();