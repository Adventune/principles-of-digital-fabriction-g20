const express = require('express');
const app = express();
const port = 3000;

app.get('/', (req, res) => {
    res.sendFile(__dirname + '/index.html');
});

app.get('/moisture', (req, res) => {
    res.send('50%');
});

app.listen(port, () => {
    console.log(`app listening on port ${port}`);
});
