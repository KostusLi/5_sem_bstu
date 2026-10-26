const http = require('http');
const fs = require('fs');

const info = "Хаткевич Константин Владимирович";

const server = http.createServer((request, response) => {
    if (request.url === '/fetch' && request.method === 'GET') {
        fs.readFile('fetch.html', (error, data) => {
            if (error) {
                response.writeHead(500, { 'Content-Type': 'text/plain' });
                response.end('Ошибка при чтении файла');
                return;
            }

            response.writeHead(200, { 'Content-Type': 'text/html;' });
            response.end(data);
        });
    }
    else if (request.url === '/api/name' && request.method === 'GET') {

        response.writeHead(200, {
            'Content-Type': 'text/plain; charset=utf-8'
        });

        response.end(info);

    }
});

server.listen(5000, () => {
    console.log('Сервер запущен: http://localhost:5000');
});