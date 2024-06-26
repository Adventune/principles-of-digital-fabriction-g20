// Import dependencies
import express from 'express';
import db from './db.mjs';
import path from 'path';
import bodyParser from 'body-parser';

// Constants
const PORT = 3000;

// Variables
let watering = false;

// Database init
db.init();

// Create an express app
const app = express();

// Main route, returns the index.html
app.get('/', (req, res) => {
    res.sendFile('/app/src/index.html');
});

// Middleware to parse the body of a POST request
app.use(bodyParser.text());

// Middleware to check an auth token
app.use((req, res, next) => {
    if (req.get('MarkoAuth') != 'Markonsalasana2024') res.sendStatus(401);
    else next();
});

app.use((req, res, next) => {
    res.header('Content-Type', 'text/plain');
    next();
});

// GET Routes
app.get('/moisture', (req, res) => {
    res.status(200).send(db.database['moisture'].toString());
});
app.get('/trigger', (req, res) => {
    res.status(200).send((watering ? 42069 : db.database['trigger-threshold']).toString());
    if (watering && !req.get('X-Web')) watering = false;
});

// POST Routes
app.post(
    '/set-moisture',
    (req, res) => {
        const moistureReading = parseInt(req.body);
        db.database['moisture'] = moistureReading;
        db.database['last-update'] = new Date().toISOString();
        res.status(200).send('Success');
    },
    db.save
);

app.post(
    '/set-trigger',
    bodyParser.text({ type: 'application/x-www-form-urlencoded' }),
    (req, res) => {
        const triggerLevel = parseInt(req.body.split('=')[1]);
        db.database['trigger-threshold'] = triggerLevel;
        res.status(200).send('Success');
    },
    db.save
);

app.get('/water', (req, res) => {
    watering = true;
    res.sendStatus(200);
});

app.get('/battery', (req, res) => {
    res.status(200).send('<p>Last moisture update: ' + db.database['last-update'] + '</p>');
});

// Start the server
app.listen(PORT, () => {
    console.log(`app listening on port ${PORT}`);
});
