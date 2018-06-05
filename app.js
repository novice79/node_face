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
    addon.get_face_trait(img_data, (data, trait)=>{
        b = new Date().getTime();
        winston.info(`total cost: %d ms`, b-a);
        data = JSON.parse(data);
        if(trait){
            data.trait = trait.toString('hex');
        }
        res.json(data);
    })    
});
app.post('/email', function (req, res) {
    res.end('david@cninone.com');
});
app.post('/author', function (req, res) {
    res.end('david');
});
app.post('/nickname', function (req, res) {
    res.end('novice');
});
app.post('/speak', function (req, res) {
    if (!req.body) return res.sendStatus(400);
    let data = req.body;
    // console.log(data)
    if ( !data.words ) return res.sendStatus(400);
    addon.speak(data.words, ret=>{
        res.json({
            ret
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
    addon.cmp_traits(t1_buff, t2_buff, (ret)=>{
        b = new Date().getTime();
        winston.info(`/cmp_face_by_traits total cost: %d ms`, b-a);
        res.end(ret);
    })    
});
app.post('/cmp_face_by_trait_and_img', async function (req, res) {
    let a = new Date().getTime(), b;
    if (!req.body) return res.sendStatus(400);
    let data = req.body;
    if (! (data.img && data.trait) ) return res.sendStatus(400);
    let img = data.img.split(',').pop();
    img = Buffer.from(img, "base64");
    const trait = Buffer.from(data.trait, "hex");
    addon.cmp_trait_and_img(trait, img, (ret, t)=>{
        // console.log(ret)
        b = new Date().getTime();
        console.log(`/cmp_face_by_trait_and_img total cost: %d ms`, b-a);
        winston.info(`/cmp_face_by_trait_and_img total cost: %d ms`, b-a);
        ret = JSON.parse(ret);
        if(t) ret.trait = t.toString('hex') 
        res.json(ret);
    })
});
app.post('/cmp_face_by_imgs', async function (req, res) {
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
    addon.cmp_images(img1, img2, (ret, t1, t2)=>{
        // console.log(ret)
        b = new Date().getTime();
        console.log(`/cmp_face_by_imgs total cost: %d ms`, b-a);
        winston.info(`/cmp_face_by_imgs total cost: %d ms`, b-a);
        // console.log(typeof ret);
        ret = JSON.parse(ret);
        if(t1) ret.trait1 = t1.toString('hex') 
        if(t2) ret.trait2 = t2.toString('hex') 
        res.json(ret);
    })
    // const t1 = new Promise(function(resolve, reject) {
    //     addon.get_face_trait(img1, (err, count, trait)=>{
    //         if (err) {
    //             reject(err);
    //         } else {
    //             resolve({count, trait});
    //         }
    //     })
    // });
    // let trait1 = await t1;
    // const t2 = new Promise(function(resolve, reject) {
    //     addon.get_face_trait(img2, (err, count, trait)=>{
    //         if (err) {
    //             reject(err);
    //         } else {
    //              resolve({count, trait});
    //         }
    //     })
    // });        
    // let trait2 = await t2;
    // if( !(trait1.count == 1 && trait2.count == 1 ) ){
    //     return res.json({ trait1, trait2 });
    // }
    // // winston.info(`trait1: %s`, trait1.trait.toString('hex'));
    // // winston.info(`trait2: %s`, trait2.trait.toString('hex'));
    // addon.cmp_traits(trait1.trait, trait2.trait, (err, diff)=>{
    //     b = new Date().getTime();
    //     winston.info(`/cmp_face_by_imgs total cost: %d ms`, b-a);
    //     console.log('diff='+diff);
    //     res.json({
    //         diff
    //     });
    // });
  
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