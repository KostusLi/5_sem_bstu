const sendmail = require('sendmail')({
    devHost: 'localhost',
    devPort: 1025,
});

const FIXED_EMAIL = 'beloboba@gmail.com';

function send(text) {
    return new Promise((resolve, reject) => {
        sendmail({
            from: 'kostik@gmail.com',
            to: FIXED_EMAIL,
            subject: 'Помой пол на кухне, короче, а то мне лень',
            text: text,
            html: `<p>${text}</p>`,
        }, (err, reply) => {
            if (err) {
                console.error('Ошибка отправки:', err.message);
                reject(err);
            } else {
                console.log('Письмо отправлено:', reply);
                resolve(reply);
            }
        });
    });
}

module.exports = { send };