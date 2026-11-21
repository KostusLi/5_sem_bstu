const http = require('http');
const fs = require('fs');
const path = require('path');
const url = require('url');
const querystring = require('querystring');

const PORT = 3000;

if (!fs.existsSync('static')) {
    fs.mkdirSync('static');
}

const server = http.createServer((req, res) => {
    const parsedUrl = url.parse(req.url, true);
    const pathname = parsedUrl.pathname;
    const query = parsedUrl.query;
    const method = req.method;

    console.log(`[${method}] ${pathname}`);

    // 1
    if (method === 'GET' && pathname === '/connection') {
        res.end(`Текущее значение KeepAliveTimeout: ${server.keepAliveTimeout}`);
        return;
    }

    if (method === 'GET' && pathname.startsWith('/connection/set=')) {
        const setValue = pathname.split('=')[1];
        server.keepAliveTimeout = parseInt(setValue);
        res.end(`Установлено новое значение параметра KeepAliveTimeout=${setValue}`);
        return;
    }

    // 2
    if (method === 'GET' && pathname === '/headers') {
        res.setHeader('X-My-Custom-Header', 'Hello from Lab 8');
        let responseText = 'Заголовки запроса\n';
        for (const [key, value] of Object.entries(req.headers)) {
            responseText += `${key}: ${value}\n`;
        }
        res.setHeader('Content-Type', 'text/plain; charset=utf-8');
        res.end(responseText);
        return;
    }

    // 3
    if (method === 'GET' && pathname === '/parameter' && query.x && query.y) {
        const x = parseFloat(query.x);
        const y = parseFloat(query.y);
        res.setHeader('Content-Type', 'text/plain; charset=utf-8');
        if (!isNaN(x) && !isNaN(y)) {
            res.end(`Сумма: ${x + y}\nРазность: ${x - y}\nПроизведение: ${x * y}\nЧастное: ${x / y}`);
        } else {
            res.end('Ошибка: параметры должны быть числами');
        }
        return;
    }

    // 4
    if (method === 'GET' && pathname.startsWith('/parameter/')) {
        const parts = pathname.split('/');
        if (parts.length === 4) {
            const x = parseFloat(parts[2]);
            const y = parseFloat(parts[3]);
            res.setHeader('Content-Type', 'text/plain; charset=utf-8');
            if (!isNaN(x) && !isNaN(y)) {
                res.end(`Сумма: ${x + y}\nРазность: ${x - y}\nПроизведение: ${x * y}\nЧастное: ${x / y}`);
            } else {
                res.end(`URI: ${pathname}`);
            }
        } else {
            res.end(`URI: ${pathname}`);
        }
        return;
    }

    // 5
    if (method === 'GET' && pathname === '/close') {
        res.end('Сервер будет остановлен через 10 секунд...');
        console.log('Сервер останавливается...');
        setTimeout(() => {
            process.exit(0);
        }, 10000);
        return;
    }

    // 6
    if (method === 'GET' && pathname === '/socket') {
        const clientIp = req.socket.remoteAddress;
        const clientPort = req.socket.remotePort;
        const serverIp = req.socket.localAddress;
        const serverPort = req.socket.localPort;
        res.end(`Client IP: ${clientIp}\nClient Port: ${clientPort}\nServer IP: ${serverIp}\nServer Port: ${serverPort}`);
        return;
    }

    // 7
    if (pathname === '/req-data') {
        let chunkCount = 0;
        res.setHeader('Content-Type', 'text/plain; charset=utf-8');
        req.on('data', (chunk) => {
            chunkCount++;
            console.log(`Получен чанк #${chunkCount}, размер: ${chunk.length} байт`);
        });
        req.on('end', () => {
            res.end(`Запрос обработан. Всего получено чанков: ${chunkCount}`);
        });
        return;
    }

    // 8
    if (method === 'GET' && pathname === '/resp-status') {
        const code = parseInt(query.code) || 200;
        const mess = query.mess || 'OK';
        
        res.statusCode = code;
        res.statusMessage = mess;
        res.setHeader('Content-Type', 'text/plain; charset=utf-8');
        res.end(`Статус: ${code}, Пояснение: ${mess}`);
        return;
    }

    // 9
    if (pathname === '/formparameter') {
        if (method === 'GET') {
            res.setHeader('Content-Type', 'text/html; charset=utf-8');
            res.end(`
                <form action="/formparameter" method="POST">
                    Text: <input type="text" name="textField"><br>
                    Number: <input type="number" name="numberField"><br>
                    Date: <input type="date" name="dateField"><br>
                    Checkbox: <input type="checkbox" name="checkField" value="yes"><br>
                    Radio: <input type="radio" name="radioField" value="opt1"> Opt1 
                           <input type="radio" name="radioField" value="opt2"> Opt2<br>
                    Textarea: <textarea name="textArea"></textarea><br>
                    <input type="submit" name="submitBtn" value="Submit1">
                    <input type="submit" name="submitBtn" value="Submit2">
                </form>
            `);
            return;
        } 
        if (method === 'POST') {
            let body = '';
            req.on('data', chunk => body += chunk);
            req.on('end', () => {
                const formData = querystring.parse(body);
                res.setHeader('Content-Type', 'text/plain; charset=utf-8');
                res.end('Полученные данные:\n' + JSON.stringify(formData, null, 2));
            });
            return;
        }
    }

    // 10
    if (method === 'POST' && pathname === '/json') {
        let body = '';
        req.on('data', chunk => body += chunk);
        req.on('end', () => {
            try {
                const data = JSON.parse(body);
                const responseObj = {
                    "__comment": "Лабка 8, №10",
                    "x_plus_y": (data.x || 0) + (data.y || 0),
                    "Concatenation_s_o": `Сообщение: ${data.s || ''}, ${data.o ? data.o.name || '' : ''}`,
                    "Length_m": data.m && Array.isArray(data.m) ? data.m.length : 0
                };
                res.setHeader('Content-Type', 'application/json; charset=utf-8');
                res.end(JSON.stringify(responseObj, null, 2));
            } catch (e) {
                res.writeHead(400);
                res.end('Invalid JSON');
            }
        });
        return;
    }

    // 11
    if (method === 'POST' && pathname === '/xml') {
        let body = '';
        req.on('data', chunk => body += chunk);
        req.on('end', () => {
            try {
                const idMatch = body.match(/id\s*=\s*"([^"]+)"/);
                const requestId = idMatch ? idMatch[1] : 'unknown';
                const xValues = [...body.matchAll(/<x\s+value\s*=\s*"([^"]+)"/g)].map(m => parseFloat(m[1]));
                const mValues = [...body.matchAll(/<m\s+value\s*=\s*"([^"]+)"/g)].map(m => m[1]);
                const sumX = xValues.reduce((a, b) => a + b, 0);
                const concatM = mValues.join('');
                const responseId = Date.now();
                const xmlResponse = `<response id="${responseId}" request="${requestId}">
                    <sum element="x" result="${sumX}" />
                    <concat element="m" result="${concatM}" />
                    </response>`;
                res.setHeader('Content-Type', 'application/xml; charset=utf-8');
                res.end(xmlResponse);
            } catch (e) {
                res.writeHead(400);
                res.end('Invalid XML');
            }
        });
        return;
    }

    // 12
    if (method === 'GET' && pathname === '/files') {
        fs.readdir('static', (err, files) => {
            if (err) {
                res.writeHead(500);
                res.end('Server Error');
                return;
            }
            res.setHeader('X-static-files-count', files.length);
            res.end(`Количество файлов в static: ${files.length}`);
        });
        return;
    }

    // 13
    if (method === 'GET' && pathname.startsWith('/files/') && pathname !== '/files/') {
        const filename = pathname.replace('/files/', '');
        const filePath = path.join(__dirname, 'static', filename);

        fs.stat(filePath, (err, stats) => {
            if (err || !stats.isFile()) {
                res.writeHead(404);
                res.end('File Not Found');
                return;
            }
            const ext = path.extname(filename);
            let mimeType = 'application/octet-stream';
            if (ext === '.txt') mimeType = 'text/plain';
            if (ext === '.html') mimeType = 'text/html';
            if (ext === '.jpg' || ext === '.jpeg') mimeType = 'image/jpeg';
            if (ext === '.png') mimeType = 'image/png';

            res.writeHead(200, { 'Content-Type': mimeType });
            fs.createReadStream(filePath).pipe(res);
        });
        return;
    }

    // 14
    if (pathname === '/upload') {
        if (method === 'GET') {
            res.setHeader('Content-Type', 'text/html; charset=utf-8');
            res.end(`
                <form action="/upload" method="POST" enctype="multipart/form-data">
                    <input type="file" name="file">
                    <input type="submit" value="Upload">
                </form>
            `);
            return;
        }

        if (method === 'POST') {
            let chunks = [];
            req.on('data', chunk => chunks.push(chunk));
            req.on('end', () => {
                const buffer = Buffer.concat(chunks);
                const boundary = req.headers['content-type'].split('boundary=')[1];
                
                const boundaryBuffer = Buffer.from('--' + boundary);
                const endBoundaryBuffer = Buffer.from('--' + boundary + '--');
                
                const start = buffer.indexOf(boundaryBuffer) + boundaryBuffer.length;
                const end = buffer.indexOf(endBoundaryBuffer);
                
                if (start === -1 || end === -1) {
                    res.end('Ошибка парсинга');
                    return;
                }

                const fileData = buffer.slice(start, end);
                
                const headerEnd = fileData.indexOf(Buffer.from('\r\n\r\n'));
                if (headerEnd === -1) {
                     res.end('Ошибка: не найден разделитель заголовков');
                     return;
                }

                const headersStr = fileData.slice(0, headerEnd).toString();
                const fileNameMatch = headersStr.match(/filename="([^"]+)"/);
                let fileName = fileNameMatch ? fileNameMatch[1] : 'uploaded_file.bin';

                let fileContent = fileData.slice(headerEnd + 4);
                
                if (fileContent.slice(-2).toString() === '\r\n') {
                    fileContent = fileContent.slice(0, -2);
                }

                fs.writeFileSync(path.join(__dirname, 'static', fileName), fileContent);
                res.end(`File ${fileName} uploaded successfully!`);
            });
            return;
        }
    }

    //9
    if (method === 'GET' && pathname === '/download') {
        const filePath = path.join(__dirname, 'MyFile.txt'); // или любой другой файл
        fs.stat(filePath, (err, stats) => {
            if (err) {
                res.writeHead(404);
                res.end('File Not Found');
                return;
            }
            res.writeHead(200, {
                'Content-Type': 'application/octet-stream', // Отдаем как файл
                'Content-Disposition': 'attachment; filename="MyFile.txt"',
                'Content-Length': stats.size
            });
            fs.createReadStream(filePath).pipe(res);
        });
        return;
    }

    res.writeHead(404, { 'Content-Type': 'text/plain; charset=utf-8' });
    res.end('404 Not Found');
});

server.listen(PORT, () => {
    console.log(`Сервер запущен на http://localhost:${PORT}`);
});