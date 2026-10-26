const http = require('http');

const PORT = 3000;

const server = http.createServer((req, res)=>
{
    const method = req.method;

    const url = req.url;

    let body = '';

    req.on('data', (chunk)=>
    {
        body+=chunk.body;
    });

    req.on('end', ()=>
    {
        const htmlResponse = `
            <!DOCTYPE html>
            <html>
            <head>
                <meta charset="utf-8">
                <title>Request Info</title>
            </head>
            <body>
                <h1>Информация о запросе</h1>
                <p><strong>Метод запроса (Method):</strong> ${method}</p>
                <p><strong>URI запроса:</strong> ${url}</p>
                <p><strong>Тело запроса (Body):</strong> ${body || '<em>Нет данных</em>'}</p>
            </body>
            </html>
        `;

        res.writeHead(200, {'Content-Type':'text/html'});
        res.end(htmlResponse);
    })

})


server.listen(PORT, ()=>
{
    console.log(`BamBamBam: http://localhost:${PORT}`);
})