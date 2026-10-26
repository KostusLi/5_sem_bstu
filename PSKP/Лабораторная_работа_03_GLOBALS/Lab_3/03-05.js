const http = require('http');

function factorialAsync(n, cb) {
    if (n === 0 || n === 1) {
        setImmediate(() => {
            cb(1);
        });
    } else {
        setImmediate(() => {
            factorialAsync(n - 1, (result) => {
                cb(n * result);
            });
        });
    }
}

const htmlPage = `
<!DOCTYPE html>
<html>
<head>
    <meta charset="utf-8">
    <title>Факториалы (setImmediate)</title>
</head>
<body>
    <div id="results"></div>
    <script>
        const resultsDiv = document.getElementById('results');
        const startTime = Date.now();
        let promises = [];
        
        for (let x = 1; x <= 20; x++) {
            const p = document.createElement('div');
            p.id = 'res-' + x;
            resultsDiv.appendChild(p);

            promises.push(
                fetch('/fact?k=' + x)
                .then(response => response.json())
                .then(data => {
                    const t = Date.now() - startTime;
                    document.getElementById('res-' + x).innerHTML = 
                        x + '. Результат: ' + t + '-' + data.k + '/' + data.fact;
                })
            );
        }

        Promise.all(promises).then(() => {
            const totalTime = Date.now() - startTime;
            const total = document.createElement('div');
            total.innerHTML = '<br><b>Общая продолжительность цикла: ' + totalTime + ' мс</b>';
            resultsDiv.appendChild(total);
        });
    </script>
</body>
</html>
`;

const server = http.createServer((request, response) => {
    const reqUrl = new URL(request.url, `http://${request.headers.host}`);

    if (request.method === 'GET' && reqUrl.pathname === '/') {
        response.writeHead(200, { 'Content-Type': 'text/html; charset=utf-8' });
        response.end(htmlPage);
    } 
    else if (request.method === 'GET' && reqUrl.pathname === '/fact') {
        const kParam = reqUrl.searchParams.get('k');

        if (kParam !== null && !isNaN(kParam)) {
            const k = parseInt(kParam, 10);
            
            factorialAsync(k, (factValue) => {
                response.writeHead(200, { 'Content-Type': 'application/json; charset=utf-8' });
                response.end(JSON.stringify({ k: k, fact: factValue }));
            });
            
        } else {
            response.writeHead(400, { 'Content-Type': 'text/plain; charset=utf-8' });
            response.end('Bad Request');
        }
    } 
    else {
        response.writeHead(404, { 'Content-Type': 'text/plain; charset=utf-8' });
        response.end('Not Found');
    }
});

server.listen(5000, () => {
    console.log('Server (setImmediate) running at http://localhost:5000/');
});