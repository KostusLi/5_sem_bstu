const http = require('http');
const fs = require('fs');
const path = require('path');
const readline = require('readline');
const DB = require('./db');

const db = new DB();

db.on('ready',  () => console.log('[EVENT] ready'));
db.on('select', (rows) => console.log('[EVENT] select, строк:', rows.length));
db.on('insert', (row)  => console.log('[EVENT] insert:', row));
db.on('update', (row)  => console.log('[EVENT] update:', row));
db.on('delete', (row)  => console.log('[EVENT] delete:', row));
db.on('COMMIT', (n)    => console.log('[EVENT] COMMIT #' + n));
db.on('error',  (err)  => console.error('[EVENT] error:', err.message));

const PORT = 5000;

let requestCount = 0;

const stats = {
    active: false,
    start: null,
    finish: null,
    requests: 0,
    commits: 0
};

db.on('COMMIT', () => {
    if (stats.active) stats.commits++;
});

function nowISO() {
    const d = new Date();
    const pad = (n) => String(n).padStart(2, '0');
    return `${d.getFullYear()}-${pad(d.getMonth()+1)}-${pad(d.getDate())} ` +
           `${pad(d.getHours())}:${pad(d.getMinutes())}:${pad(d.getSeconds())}`;
}

function sendJSON(res, statusCode, data) {
    res.writeHead(statusCode, {
        'Content-Type': 'application/json; charset=utf-8'
    });
    res.end(JSON.stringify(data));
}

function readBody(req) {
    return new Promise((resolve, reject) => {
        let body = '';
        req.on('data', chunk => body += chunk.toString());
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

function sendHTML(res, filename) {
    const filePath = path.join(__dirname, filename);
    fs.readFile(filePath, (err, data) => {
        if (err) {
            res.writeHead(404, { 'Content-Type': 'text/plain; charset=utf-8' });
            res.end('Файл не найден: ' + filename);
            return;
        }
        res.writeHead(200, { 'Content-Type': 'text/html; charset=utf-8' });
        res.end(data);
    });
}

const server = http.createServer(async (req, res) => {
    const parsedUrl = new URL(req.url, `http://${req.headers.host}`);
    const pathname = parsedUrl.pathname;

    console.log(`[${req.method}] ${req.url}`);

    if (req.method === 'GET' && pathname === '/') {
        sendHTML(res, 'index.html');
        return;
    }

    if (req.method === 'GET' && pathname === '/api/ss') {
        sendJSON(res, 200, {
            start:  stats.start || '',
            finish: stats.active ? '' : (stats.finish || ''),
            request: stats.requests,
            commit:  stats.commits
        });
        return;
    }

    if (pathname !== '/api/db') {
        sendJSON(res, 404, { error: 'Not found' });
        return;
    }

    requestCount++;
    if (stats.active) stats.requests++;

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
            const body = await readBody(req);
            const id = parseInt(body.id, 10);

            if (!id) {
                sendJSON(res, 400, { error: 'Поле id обязательно' });
                return;
            }

            const updatedRow = await db.update(id, body.name, body.bday);
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
    console.log(`API статистики: http://localhost:${PORT}/api/ss`);
    console.log('Команды: sd [x] | sc [x] | ss [x]');
});


let shutdownTimer = null;
let commitTimer = null;
let statsStopTimer = null;

function cmdSd(seconds) {
    if (shutdownTimer) {
        clearTimeout(shutdownTimer);
        shutdownTimer = null;
        console.log('[CMD] Предыдущая команда sd отменена');
    }

    if (seconds == null) {
        console.log('[CMD] Остановка сервера отменена');
        return;
    }

    const ms = seconds * 1000;
    shutdownTimer = setTimeout(() => {
        console.log(`\n[CMD] Сервер останавливается...`);
        server.close(() => {
            console.log('[CMD] Сервер остановлен. Выход.');
            process.exit(0);
        });
    }, ms);

    console.log(`[CMD] Сервер будет остановлен через ${seconds} сек.`);
}

function cmdSc(seconds) {
    if (commitTimer) {
        clearInterval(commitTimer);
        commitTimer = null;
        console.log('[CMD] Периодический commit остановлен');
    }

    if (seconds == null) {
        return;
    }

    const ms = seconds * 1000;
    commitTimer = setInterval(() => {
        db.commit();
    }, ms);

    commitTimer.unref();

    console.log(`[CMD] Периодический commit запущен каждые ${seconds} сек.`);
}

function cmdSs(seconds) {
    if (stats.active) {
        if (statsStopTimer) {
            clearTimeout(statsStopTimer);
            statsStopTimer = null;
        }
        stats.active = false;
        stats.finish = nowISO();
        console.log(`[CMD] Сбор статистики остановлен. Итог: ` +
                    `requests=${stats.requests}, commits=${stats.commits}`);
    }

    if (seconds == null) {
        return;
    }

    stats.active = true;
    stats.start = nowISO();
    stats.finish = null;
    stats.requests = 0;
    stats.commits = 0;

    console.log(`[CMD] Сбор статистики запущен на ${seconds} сек.`);

    statsStopTimer = setTimeout(() => {
        stats.active = false;
        stats.finish = nowISO();
        statsStopTimer = null;
        console.log(`[CMD] Сбор статистики завершён. Итог: ` +
                    `requests=${stats.requests}, commits=${stats.commits}`);
    }, seconds * 1000);

    statsStopTimer.unref();
}

function handleCommand(line) {
    const parts = line.trim().split(/\s+/);
    const cmd = parts[0];
    const argStr = parts[1];

    let arg = null;
    if (argStr !== undefined) {
        const num = Number(argStr);
        if (!isNaN(num) && num > 0) {
            arg = num;
        }
    }

    switch (cmd) {
        case 'sd': cmdSd(arg); break;
        case 'sc': cmdSc(arg); break;
        case 'ss': cmdSs(arg); break;
        case '':   break;
        default:
            console.log(`[CMD] Неизвестная команда: ${cmd}`);
    }
}

const rl = readline.createInterface({
    input: process.stdin,
    output: process.stdout,
    terminal: false
});

rl.on('line', (line) => {
    handleCommand(line);
});