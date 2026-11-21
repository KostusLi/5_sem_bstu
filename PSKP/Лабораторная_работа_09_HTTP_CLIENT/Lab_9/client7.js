const http = require('http');
const fs = require('fs');

const boundary = '----MyBoundary987654321';
const fileContent = fs.readFileSync('MyFile.png'); // Файл должен существовать

const header = `--${boundary}\r\n`;
const contentDisposition = `Content-Disposition: form-data; name="file"; filename="MyFile.png"\r\n`;
const contentType = `Content-Type: image/png\r\n\r\n`;
const footer = `\r\n--${boundary}--\r\n`;

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
    console.log('=== Задание 07 ===');
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