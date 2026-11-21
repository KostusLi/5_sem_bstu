const http = require('http');
const { createStaticHandler } = require('./m07-01');

const PORT = 3000;
const handler = createStaticHandler('static');

const server = http.createServer(handler);

server.listen(PORT, () => {
  console.log(`Server: http://localhost:${PORT}/`);
});