const http = require('http');

const postData = JSON.stringify({
    x: 1,
    y: 2,
    s: 'Иванов, Иван',
    o: { name: 'Иван' },
    m: ['a', 'b', 'c', 'd']
});

const options = {
    hostname: 'localhost',
    port: 3000,
    path: '/json',
    method: 'POST',
    headers: {
        'Content-Type': 'application/json',
        'Content-Length': Buffer.byteLength(postData)
    }
};

const req = http.request(options, (res) => {
    console.log('=== Задание 04 ===');
    console.log(`Статус: ${res.statusCode}`);
    
    let data = '';
    res.on('data', chunk => data += chunk);
    res.on('end', () => {
        try {
            const jsonResponse = JSON.parse(data);
            console.log('Ответ (распарсенный):');
            console.log(JSON.stringify(jsonResponse, null, 2));
        } catch (e) {
            console.log(`Тело ответа (сырое): ${data}`);
        }
    });
});

req.on('error', (e) => console.error(`Ошибка: ${e.message}`));
req.write(postData);
req.end();