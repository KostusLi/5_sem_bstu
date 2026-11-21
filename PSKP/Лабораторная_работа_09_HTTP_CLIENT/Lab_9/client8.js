const http = require('http');
const fs = require('fs');

const options = {
    hostname: 'localhost',
    port: 3000,
    path: '/download',
    method: 'GET'
};

const req = http.request(options, (res) => {
    console.log('=== Задание 08 ===');
    console.log(`Статус: ${res.statusCode}`);
    
    // Сохраняем входящий поток в файл
    const fileStream = fs.createWriteStream('DownloadedFile.txt');
    
    res.on('data', (chunk) => {
        fileStream.write(chunk);
    });
    
    res.on('end', () => {
        fileStream.end();
        console.log('Файл DownloadedFile.txt успешно получен и сохранен!');
    });
});

req.on('error', (e) => console.error(`Ошибка: ${e.message}`));
req.end();