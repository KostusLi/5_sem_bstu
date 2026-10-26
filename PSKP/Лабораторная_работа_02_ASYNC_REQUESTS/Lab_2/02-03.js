const http = require('http');

const info = "Хаткевич Константин Владимирович";

const server = http.createServer((request, response) => {
            
    if(request.url==="/api/name" && request.method==='GET')
    {
        response.writeHead(200, { 'Content-Type': 'text/plain; charset=utf-8' });
        response.end(info);
    }
});

server.listen(5000, () => {
    console.log('Сервер запущен: http://localhost:5000');
});