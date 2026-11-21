const http = require('http');

const options = {
    hostname: 'localhost',
    port: 3000,
    path: '/connection',
    method: 'GET'
};

const req = http.request(options, (res) => {
    console.log('=== Задание 01 ===');
    console.log(`Статус: ${res.statusCode}`);
    console.log(`Сообщение к статусу: ${res.statusMessage}`);
    console.log(`IP сервера: ${res.socket.remoteAddress}`);
    console.log(`Порт сервера: ${res.socket.remotePort}`);
    
    let data = '';
    res.on('data', chunk => data += chunk);
    res.on('end', () => {
        console.log(`Тело ответа: ${data}`);
    });
});

req.on('error', (e) => console.error(`Ошибка: ${e.message}`));
req.end();