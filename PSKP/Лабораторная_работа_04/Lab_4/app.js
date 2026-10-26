const http = require('http');
const url = require('url');
const DB = require('./db');

const db = new DB();

db.on('ready',  () => console.log('[EVENT] ready'));
db.on('select', (rows) => console.log('[EVENT] select, строк:', rows.length));
db.on('insert', (row)  => console.log('[EVENT] insert:', row));
db.on('update', (row)  => console.log('[EVENT] update:', row));
db.on('delete', (row)  => console.log('[EVENT] delete:', row));
db.on('error',  (err)  => console.error('[EVENT] error:', err.message));

const PORT = 5000;

function sendJSON(res, statusCode, data) {
    res.writeHead(statusCode, {
        'Content-Type': 'application/json; charset=utf-8'
    });
    res.end(JSON.stringify(data));
}

function readBody(req) {
    return new Promise((resolve, reject) => {
        let body = '';
        req.on('data', chunk => {
            body += chunk.toString();
        });
        req.on('end', () => {
            try {
                resolve(body ? JSON.parse(body) : {});
            } catch (err) {
                reject(err);
            }
        });
        req.on('error', reject);
    });
}

const server = http.createServer(async (req, res) => {
    const parsedUrl = new URL(req.url, `http://${req.headers.host}`);
    const pathname = parsedUrl.pathname;
    const query = parsedUrl.query;

    console.log(`[${req.method}] ${req.url}`);

    if (pathname !== '/api/db') {
        sendJSON(res, 404, { error: 'Not found' });
        return;
    }

    try {
        if (req.method === 'GET') {
            const rows = await db.select();
            sendJSON(res, 200, rows);
            return;
        }

        if (req.method === 'POST') {
            const { name, bday } = await readBody(req);
            if (!name || !bday) {
                sendJSON(res, 400, { error: 'Поля name и bday обязательны' });
                return;
            }
            const newRow = await db.insert(name, bday);
            sendJSON(res, 201, newRow);
            return;
        }

        if (req.method === 'PUT') {
            const { id, name, bday } = await readBody(req);
            if (!id || !name || !bday) {
                sendJSON(res, 400, { error: 'Поля id, name и bday обязательны' });
                return;
            }
            const updatedRow = await db.update(id, name, bday);
            if (!updatedRow) {
                sendJSON(res, 404, { error: `Строка с id=${id} не найдена` });
                return;
            }
            sendJSON(res, 200, updatedRow);
            return;
        }

        if (req.method === 'DELETE') {
            const id = parseInt(parsedUrl.searchParams.get('id'), 10);
            if (!id) {
                sendJSON(res, 400, { error: 'Query-параметр id обязателен' });
                return;
            }
            const deletedRow = await db.delete(id);
            if (!deletedRow) {
                sendJSON(res, 404, { error: `Строка с id=${id} не найдена` });
                return;
            }
            sendJSON(res, 200, deletedRow);
            return;
        }

        sendJSON(res, 405, { error: 'Method not allowed' });
    } catch (err) {
        console.error('Ошибка:', err);
        sendJSON(res, 500, { error: 'Внутренняя ошибка сервера' });
    }
});

server.listen(PORT, () => {
    console.log(`Сервер запущен: http://localhost:${PORT}`);
});