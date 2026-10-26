const http = require('http');

const ALLOWED_STATES = ['norm', 'stop', 'test', 'idle'];

let currentState = 'norm';

const server = http.createServer((request, response) => {
    response.writeHead(200, { 'Content-Type': 'text/html; charset=utf-8' });
    
    response.end(`<h1>${currentState}</h1>`);
});

server.listen(5000, () => {
    console.log('Server running at http://localhost:5000/');
    process.stdout.write(`${currentState}->`);
});


process.stdin.setEncoding('utf-8');

process.stdin.on('data', (data) => {
    const input = data.trim();

    if (input === 'exit') {
        process.exit(0);
    } 
    else if (ALLOWED_STATES.includes(input)) {
        console.log(`reg = ${currentState}--> ${input}`);
        currentState = input;
    } 
    else {
        console.log(input);
    }

    process.stdout.write(`${currentState}->`);
});