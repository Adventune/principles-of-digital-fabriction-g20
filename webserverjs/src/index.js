const express = require('express');
const app = express();
const port = 3000;

const fs = require('fs')


let currentMoisture = 0;
let database = {
    "trigger-threshold": 0
};
let wateringMode = false;

app.use(express.json())

const tokenChecker = (req, res, next) => {
    if(req.get("MarkoAuth") != "Markonsalasana2024")
        res.sendStatus(401)
    else
        next()
}
 
fs.readFile("src/database.json", (err, data) => { 
    if (err) throw err; 
    database = JSON.parse(data); 
});

const writeToDb = () => fs.writeFile(
    "src/database.json",
    JSON.stringify(database),
    err => { 
        if (err) throw err;
        console.log("Success")
});

const saveToDb = (req,res,next) => {
    writeToDb()
    next()
}

app.get('/', (req, res) => {
    res.sendFile(__dirname + '/index.html');
});

app.get('/moisture', tokenChecker, (req, res) => {
    res.send("" + currentMoisture);
});

app.post('/update-moisture', tokenChecker, (req, res) => {
    if(typeof req.body.moistureReading !== 'number')
        res.sendStatus('404')
    else {
        currentMoisture = req.body.moistureReading
        console.log(currentMoisture)
        res.status(200).send("Success");
    }
});


app.get('/trigger', tokenChecker, (req, res) => {
    res.send(database["trigger-threshold"] + "")
});

app.post('/set-trigger', tokenChecker, (req, res,next) => {
    if(typeof req.body.triggerLevel !== 'number')
        res.sendStatus(404)
    else{
        console.log(database)
        database["trigger-threshold"] = req.body.triggerLevel
        console.log(database)
        res.status(200).send("Success");
        next()
    }
}, saveToDb);

app.get('/watering-mode', tokenChecker, (req,res) => {
    res.send(wateringMode)
    wateringMode = false
})
app.get('/set-watering-mode', tokenChecker, (req,res) => {
    wateringMode = req.body.wateringMode
    res.sendStatus(200)
})

app.listen(port, () => {
    console.log(`app listening on port ${port}`);
});
