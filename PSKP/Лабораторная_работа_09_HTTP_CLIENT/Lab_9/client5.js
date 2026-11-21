const http = require('http');

const postData = `<request id="28">
    <x value="1"/>
    <x value="2"/>
    <m value="a"/>
    <m value="b"/>
    <m value="c"/>
</request>`;

const options = {
    hostname: 'localhost',
    port: 3000,
    path: '/xml',
    method: 'POST',
    headers: {
        'Content-Type': 'application/xml',
        'Content-Length': Buffer.byteLength(postData)
    }
};

const req = http.request(options, (res) => {
    console.log('=== Задание 05 ===');
    console.log(`Статус: ${res.statusCode}`);
    
    let data = '';
    res.on('data', chunk => data += chunk);
    res.on('end', () => {
        console.log(`Тело ответа (XML):\n${data}`);
    });
});

req.on('error', (e) => console.error(`Ошибка: ${e.message}`));
req.write(postData);
req.end();