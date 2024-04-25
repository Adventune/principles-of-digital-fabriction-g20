// "Database" for the webapp (no need for a real database for this project)
// Imports
const fs = require('fs');

// Create a "database" for the webapp
export let database = {
    moisture: 0,
    'trigger-threshold': 0,
};

const writeDBToFile = () =>
    fs.writeFile('src/database.json', JSON.stringify(database), (err) => {
        if (err) throw err;
        console.log('Success');
    });

export function save(req, res, next) {
    writeDBToFile();
    next();
}

// Read the database from the file
export function init() {
    fs.readFile('src/database.json', (err, data) => {
        if (err) throw err;
        database = JSON.parse(data);
    });
}
