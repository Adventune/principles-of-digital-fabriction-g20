// "Database" for the webapp (no need for a real database for this project)
// Imports
import fs from 'fs';

// Create a "database" for the webapp
let database = {
    moisture: 0,
    'trigger-threshold': 0,
    'last-update': null,
};

const writeDBToFile = () =>
    fs.writeFile('src/database.json', JSON.stringify(database), (err) => {
        if (err) throw err;
        console.log('Success');
    });

function save(req, res, next) {
    writeDBToFile();
    next();
}

// Read the database from the file
function init() {
    fs.readFile('src/database.json', (err, data) => {
        if (err) throw err;
        database = JSON.parse(data);
    });
}

export default { database, save, init };
