const http = require('http');

function factorial(n) {
    if (n === 0 || n === 1) {
        return 1;
    }
    return n * factorial(n - 1);
}

const server = http.createServer((request, response) => {
    const reqUrl = new URL(request.url, `http://${request.headers.host}`);

    if (request.method === 'GET' && reqUrl.pathname === '/fact') {
        
        const kParam = reqUrl.searchParams.get('k');

        if (kParam !== null && !isNaN(kParam)) {
            const k = parseInt(kParam, 10);
            
            const factValue = factorial(k);

            const result = {
                k: k,
                fact: factValue
            };

            response.writeHead(200, { 'Content-Type': 'application/json; charset=utf-8' });
            response.end(JSON.stringify(result));
        } else {
            response.writeHead(400, { 'Content-Type': 'text/plain; charset=utf-8' });
            response.end('Bad Request: Parameter k is missing or invalid');
        }
    } else {
        response.writeHead(404, { 'Content-Type': 'text/plain; charset=utf-8' });
        response.end('Not Found');
    }
});

server.listen(5000, () => {
    console.log('Server running at http://localhost:5000/');
});