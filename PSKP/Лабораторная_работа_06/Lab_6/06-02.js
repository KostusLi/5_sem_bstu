const http = require('http');
const querystring = require('querystring');
const sendmail = require('sendmail')({
    devHost: 'localhost',
    devPort: 1025
});

const PORT = 3000;

const server = http.createServer((req, res) => {
    if (req.method === 'GET' && req.url === '/') {
        res.writeHead(200, { 'Content-Type': 'text/html; charset=utf-8' });
        res.end(`
            <!DOCTYPE html>
            <html>
            <head><meta charset="utf-8"><title>Лаба 06-02</title></head>
            <body>
                <h1>Отправка письма</h1>
                <form method="POST" action="/send">
                    <p>От кого (email): <input type="email" name="from" required></p>
                    <p>Кому (email): <input type="email" name="to" required></p>
                    <p>Сообщение:<br><textarea name="message" rows="5" cols="40" required></textarea></p>
                    <button type="submit">Отправить</button>
                </form>
            </body>
            </html>
        `);
    }
    else if (req.method === 'POST' && req.url === '/send') {
        let body = '';
        req.on('data', chunk => { body += chunk.toString(); });
        req.on('end', () => {
            const parsed = querystring.parse(body);
            const { from, to, message } = parsed;

            sendmail({
                from: from,
                to: to,
                subject: 'Тест 06-02',
                html: message,
            }, (err, reply) => {
                res.writeHead(200, { 'Content-Type': 'text/html; charset=utf-8' });
                if (err) {
                    console.error('Ошибка отправки:', err);
                    res.end(`<h1>Ошибка</h1><p>${err.message}</p><a href="/">Назад</a>`);
                } else {
                    console.log('Письмо отправлено:', reply);
                    res.end(`<h1>Успех!</h1><p>Письмо поставлено в очередь на отправку.</p><a href="/">Назад</a>`);
                }
            });
        });
    } else {
        res.writeHead(404);
        res.end('Not Found');
    }
});

server.listen(PORT, () => {
    console.log(`Сервер запущен: http://localhost:${PORT}`);
});