const http = require('http');
const fs = require('fs');

const boundary = '----MyBoundary123456789';
const fileContent = fs.readFileSync('MyFile.txt');

const header = `--${boundary}\r\n`;
const contentDisposition = `Content-Disposition: form-data; name="file"; filename="MyFile.txt"\r\n`;
const contentType = `Content-Type: text/plain\r\n\r\n`;
const footer = `\r\n--${boundary}--\r\n`;

// Собираем всё в один буфер
const parts = [
    Buffer.from(header + contentDisposition + contentType),
    fileContent,
    Buffer.from(footer)
];
const postData = Buffer.concat(parts);

const options = {
    hostname: 'localhost',
    port: 3000,
    path: '/upload',
    method: 'POST',
    headers: {
        'Content-Type': `multipart/form-data; boundary=${boundary}`,
        'Content-Length': postData.length
    }
};

const req = http.request(options, (res) => {
    console.log('=== Задание 06 ===');
    console.log(`Статус: ${res.statusCode}`);
    
    let data = '';
    res.on('data', chunk => data += chunk);
    res.on('end', () => {
        console.log(`Тело ответа: ${data}`);
    });
});

req.on('error', (e) => console.error(`Ошибка: ${e.message}`));
req.write(postData);
req.end();