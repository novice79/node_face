const fs = require('fs');
const request = require('request');
const app = require('express')();
const bodyParser = require('body-parser')
const server = require('http').Server(app);
const io = require('socket.io')(server);
const _ = require("lodash");
const moment = require('moment');
const wins = require('winston');
const addon = require('bindings')('addon');

const logDir = 'log', port = 12345;
// Create the log directory if it does not exist
if (!fs.existsSync(logDir)) {
    fs.mkdirSync(logDir);
}
global.winston = new (wins.Logger)({
    transports: [
        new (require('winston-daily-rotate-file'))({
            filename: 'vs-%DATE%.log',
            dirname: logDir,
            datePattern: 'YYYY-MM-DD',
            zippedArchive: true,
            maxSize: '20m',
            maxFiles: '14d'
        })
    ]
});
app.use(bodyParser.json());       // to support JSON-encoded bodies
app.use(bodyParser.urlencoded({     // to support URL-encoded bodies
    extended: true
}));

app.use(require('express').static('./public'));
process.on('uncaughtException', async (err) => {
    winston.error('uncaughtException', util.stringifyError(err, null, 4));
});
server.listen(port, () => {
  console.log(`express server listen on ${port}`);
  winston.info(`express server listen on ${port}`);
});
io.on('connection', function (socket) {
  socket.on('punch', (data) => {
    console.log("-----------------------打孔-----------------------");
  });
});
addon.startVideo( (o_buff, f_buff, count, trait)=> {
  const o_frame = `data:image/jpeg;base64,${o_buff.toString('base64')}`; 
  const f_frame = `data:image/jpeg;base64,${f_buff.toString('base64')}`; 
  const traits = trait.toString('hex'); 
  io.emit('video_frame', {o_frame, f_frame, count, traits});
});