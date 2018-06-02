// const addon = require('bindings')('addon');

// const worker = new addon.NativeWorker();
// worker.on('stage0', data=>{
//     console.log('on stage0, get data='+data)
// })
// setInterval(()=>{
//     console.log('in nodejs errand')
// }, 5000)
const moment = require('moment');
var winston = require('winston');
require('winston-daily-rotate-file');

// { error: 0, warn: 1, info: 2, verbose: 3, debug: 4, silly: 5 }
var transport = new (winston.transports.DailyRotateFile)({
    filename: 'application-%DATE%.log',
    datePattern: 'YYYY-MM-DD',
    level: 'silly',
    zippedArchive: true,
    timestamp: ()=> {
        return new moment().format("YYYY-MM-DD HH:mm:ss.SSS");
    },
    maxSize: '20m',
    maxFiles: '14d'
});

transport.on('rotate', function (oldFilename, newFilename) {
    // do something fun
});

var logger = new (winston.Logger)({
    transports: [
        transport
    ]
});

logger.info('Hello World!');
logger.debug('Hello new World!');
logger.silly('Hello silly World!');