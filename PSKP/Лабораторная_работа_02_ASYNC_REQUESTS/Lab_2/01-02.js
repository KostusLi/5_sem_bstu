const http = require('http');
const fs = require('fs');

const server = http.createServer((request, response) => {
    if (request.url === '/html' && request.method === 'GET') {
        fs.readFile('index.html', (error, data) => {
            if (error) {
                response.writeHead(500, { 'Content-Type': 'text/plain' });
                response.end('Ошибка при чтении файла');
                return;
            }

            response.writeHead(200, { 'Content-Type': 'text/html' });
            response.end(data);
        });
    }
});

server.listen(5000, () => {
    console.log('Сервер запущен: http://localhost:5000');
});