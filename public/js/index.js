

const sock = io('/', { transports: ['websocket'] });
sock.on('connect', function () {
    console.log('on connect to local server')    
});
const o_frame = document.getElementById("o_frame");
const f_frame = document.getElementById("f_frame");
sock.on('video_frame', function (data) {
    o_frame.src = data.o_frame;
    f_frame.src = data.f_frame;
});
