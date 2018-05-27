const addon = require('bindings')('addon');

const worker = new addon.NativeWorker();
worker.on('stage0', data=>{
    console.log('on stage0, get data='+data)
})
setInterval(()=>{
    console.log('in nodejs errand')
}, 5000)