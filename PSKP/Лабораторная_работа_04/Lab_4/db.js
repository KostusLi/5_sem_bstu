// db.js
const EventEmitter = require('events');

class DB extends EventEmitter {
    constructor() {
        super();

        this.rows = [];

        this.nextId = 1;

        setImmediate(() => {
            console.log('[DB] База данных инициализирована');
            this.emit('ready');
        });
    }

    select() {
        return new Promise((resolve) => {
            setImmediate(() => {
                const result = [...this.rows];
                this.emit('select', result);
                resolve(result);
            });
        });
    }

    insert(name, bday) {
        return new Promise((resolve) => {
            setImmediate(() => {
                const newRow = {
                    id: this.nextId++,
                    name,
                    bday
                };
                this.rows.push(newRow);
                this.emit('insert', newRow);
                resolve(newRow);
            });
        });
    }

    update(id, name, bday) {
        return new Promise((resolve) => {
            setImmediate(() => {
                const row = this.rows.find(r => r.id === id);

                if (!row) {
                    return resolve(null);
                }

                row.name = name;
                row.bday = bday;
                this.emit('update', row);
                resolve(row);
            });
        });
    }

    delete(id) {
        return new Promise((resolve) => {
            setImmediate(() => {
                const index = this.rows.findIndex(r => r.id === id);

                if (index === -1) {
                    return resolve(null);
                }

                const [deletedRow] = this.rows.splice(index, 1);
                this.emit('delete', deletedRow);
                resolve(deletedRow);
            });
        });
    }
}

module.exports = DB;