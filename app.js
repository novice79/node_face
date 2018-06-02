const fs = require('fs');
const request = require('request');
const app = require('express')();
const bodyParser = require('body-parser')
const server = require('http').Server(app);
const io = require('socket.io')(server);
const _ = require("lodash");
const moment = require('moment');
const wins = require('winston');
const BB = require('bluebird');
const addon = require('bindings')('addon');
// const cpp = BB.promisifyAll(addon, {multiArgs: true});
const logDir = 'log', port = 12345;
// Create the log directory if it does not exist
if (!fs.existsSync(logDir)) {
    fs.mkdirSync(logDir);
}
// { error: 0, warn: 1, info: 2, verbose: 3, debug: 4, silly: 5 }
global.winston = new (wins.Logger)({
    transports: [
        new (require('winston-daily-rotate-file'))({
            level: 'silly',
            filename: 'vs-%DATE%.log',
            dirname: logDir,
            datePattern: 'YYYY-MM-DD',
            timestamp: ()=> {
                return new moment().format("YYYY-MM-DD HH:mm:ss.SSS");
            },
            prepend: true,
            zippedArchive: true,
            maxSize: '20m',
            maxFiles: '14d'
        })
    ]
});
app.use(bodyParser.json({limit: '50mb'}));       // to support JSON-encoded bodies
app.use(bodyParser.urlencoded({     // to support URL-encoded bodies
    limit: '50mb',
    extended: true
}));

app.use(require('express').static('./public'));
process.on('uncaughtException', async (err) => {
    winston.error('uncaughtException', util.stringifyError(err, null, 4));
});
server.listen(port, () => {
  console.log(`express server listen on ${port}`);
  winston.info(`express server listen on ${port}`);
  addon.speak('人脸识别服务已启动', ret=>{

  })
});
app.post('/get_face_trait', function (req, res) {
    let a = new Date().getTime();
    if (!req.body) return res.sendStatus(400);
    let data = req.body;
    // console.log(data)
    // winston.info(`get_face_trait: ${JSON.stringify(data)}`);
    if (!data.img) return res.sendStatus(400);
    let img_data = data.img.split(',').pop();
    img_data = Buffer.from(img_data, "base64");
    let b = new Date().getTime();
    winston.info(`prehandle image cost: %d ms`, b-a);
    // winston.info('process.env.UV_THREADPOOL_SIZE=' + process.env.UV_THREADPOOL_SIZE);
    addon.get_face_trait(img_data, (err, count, trait)=>{
        b = new Date().getTime();
        winston.info(`total cost: %d ms`, b-a);
        res.json({
            count,
            trait: trait ? trait.toString('hex') : null
        });
    })    
});
app.post('/cmp_face_by_traits', function (req, res) {
    let a = new Date().getTime(), b;
    if (!req.body) return res.sendStatus(400);
    let data = req.body;
    // console.log(data)
    if (! (data.trait1 && data.trait2) ) return res.sendStatus(400);
    const t1_buff = Buffer.from(data.trait1, "hex");
    const t2_buff = Buffer.from(data.trait2, "hex");
    addon.cmp_traits(t1_buff, t2_buff, (err, diff)=>{
        b = new Date().getTime();
        winston.info(`/cmp_face_by_traits total cost: %d ms`, b-a);
        res.json({
            diff
        });
    })    
});
app.post('/cmp_face_by_imgs', function (req, res) {
    let a = new Date().getTime(), b;
    if (!req.body) return res.sendStatus(400);
    let data = req.body;
    if (! (data.img1 && data.img2) ) return res.sendStatus(400);
    let img1 = data.img1.split(',').pop();
    img1 = Buffer.from(img1, "base64");
    let img2 = data.img2.split(',').pop();
    img2 = Buffer.from(img2, "base64");
    // fs.writeFile(`img1.jpg`, img1, ()=>{});
    // fs.writeFile(`img2.jpg`, img2, ()=>{});
    const t1 = new Promise(function(resolve, reject) {
        addon.get_face_trait(img1, (err, count, trait)=>{
            if (err) {
                reject(err);
            } else {
                 resolve({count, trait});
            }
        })
    });
    const t2 = new Promise(function(resolve, reject) {
        addon.get_face_trait(img2, (err, count, trait)=>{
            if (err) {
                reject(err);
            } else {
                 resolve({count, trait});
            }
        })
    });
    Promise.all([t1, t2]).then( values=> {
        // console.log(values);
        let trait1 = values[0];
        let trait2 = values[1];
        if( !(trait1.count == 1 && trait2.count == 1 ) ){
            return res.json({ trait1, trait2 });
        }
        // winston.info(`trait1: %s`, trait1.trait.toString('hex'));
        // winston.info(`trait2: %s`, trait2.trait.toString('hex'));
        addon.cmp_traits(trait1.trait, trait2.trait, (err, diff)=>{
            b = new Date().getTime();
            winston.info(`/cmp_face_by_imgs total cost: %d ms`, b-a);
            console.log('diff='+diff);
            res.json({
                diff
            });
        });
    });
       
});
io.on('connection', function (socket) {
  socket.on('speak', (data) => {
    addon.speak(data, ret=>{
        
    })
    winston.info(`机器说：${data}`);
  });
});
addon.startVideo( (o_buff, f_buff, count)=> {
  const o_frame = `data:image/jpeg;base64,${o_buff.toString('base64')}`; 
  const f_frame = `data:image/jpeg;base64,${f_buff.toString('base64')}`; 

  io.emit('video_frame', {o_frame, f_frame, count});
});