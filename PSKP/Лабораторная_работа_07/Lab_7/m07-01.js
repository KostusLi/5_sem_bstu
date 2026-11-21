const fs = require('fs');
const path = require('path');

const MIME_TYPES = {
  '.html': 'text/html',
  '.css':  'text/css',
  '.js':   'text/javascript',
  '.png':  'image/png',
  '.docx': 'application/msword',
  '.json': 'application/json',
  '.xml':  'application/xml',
  '.mp4':  'video/mp4',
};


function createStaticHandler(rootDir) {
  const root = path.resolve(__dirname, rootDir);

  return function handler(req, res) {
    if (req.method !== 'GET') {
      res.writeHead(405, { 'Content-Type': 'text/plain; charset=utf-8' });
      res.end('405 Method Not Allowed');
      return;
    }

    const urlPath = req.url === '/' ? '/index.html' : req.url;
    const filePath = path.join(root, urlPath);


    const ext = path.extname(filePath).toLowerCase();
    const mime = MIME_TYPES[ext];
    if (!mime) {
      return notFound(res);
    }

    fs.readFile(filePath, (err, data) => {
      if (err) {
        return notFound(res);
      }
      res.writeHead(200, {
        'Content-Type': mime,
        'Content-Length': data.length,
      });
      res.end(data);
    });
  };
}

function notFound(res) {
  res.writeHead(404, { 'Content-Type': 'text/plain; charset=utf-8' });
  res.end('404 Not Found');
}

module.exports = { createStaticHandler };