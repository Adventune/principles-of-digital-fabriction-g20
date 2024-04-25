// Import dependencies
const express = require('express');
const fs = require('fs');
const db = require('./db');

// Constants
const PORT = 3000;

// Variables
let wateringMode = false;

// Database init
db.init();

// Create an express app
const app = express();
app.use(express.json()); // for parsing application/json

// Main route, returns the index.html
app.get('/', (req, res) => {
    res.sendFile(__dirname + '/index.html');
});

// Middleware to check an auth token
app.use((req, res, next) => {
    if (req.get('MarkoAuth') != 'Markonsalasana2024') res.sendStatus(401);
    else next();
});

// GET Routes
app.get('/moisture', (req, res) => {
    res.send(db.database['moisture']);
});
app.get('/trigger', (req, res) => {
    res.send(db.database['trigger-threshold']);
});

// POST Routes
app.post('/update-moisture', (req, res) => {
    if (typeof req.body.moistureReading !== 'number') res.sendStatus('404');
    else {
        db.database['moisture'] = req.body.moistureReading;
        res.status(200).send('Success');
    }
});
app.post(
    '/set-trigger',
    (req, res) => {
        if (typeof req.body.triggerLevel !== 'number') res.sendStatus(404);
        else {
            db.database['trigger-threshold'] = req.body.triggerLevel;
            res.status(200).send('Success');
        }
    },
    db.save
);

app.get('/watering-mode', (req, res) => {
    res.send(wateringMode);
    wateringMode = false;
});

app.get('/set-watering-mode', (req, res) => {
    wateringMode = req.body.wateringMode;
    res.sendStatus(200);
});

// Start the server
app.listen(PORT, () => {
    console.log(`app listening on port ${PORT}`);
});
