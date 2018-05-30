
var frame_data, img1_data, img2_data, cur_load_type;
const sock = io('/', { transports: ['websocket'] });
sock.on('connect', function () {
    console.log('on connect to local server')
});
const o_frame = document.getElementById("o_frame");
const f_frame = document.getElementById("f_frame");
sock.on('video_frame', function (data) {
    frame_data = data;
    o_frame.src = data.o_frame;
    f_frame.src = data.f_frame;
});
function save_img(type) {
    console.log(typeof type, type)
    if (0 == type) {
        download_img($("#img1").attr('src'), 'img1.jpg');
    } else {
        download_img($("#img2").attr('src'), 'img2.jpg');
    }
}
function allowDrop(ev) {
    ev.preventDefault();
}

function drag_o(ev) {
    ev.dataTransfer.setData("type", 0);
}
function drag_f(ev) {
    ev.dataTransfer.setData("type", 1);
}
function drop(ev) {
    ev.preventDefault();
    var data = ev.dataTransfer.getData("type");
    if (0 == data) {
        img1_data = $.extend(true, {}, frame_data);
        ev.target.src = img1_data.o_frame;
    } else {
        img2_data = $.extend(true, {}, frame_data);
        ev.target.src = img2_data.f_frame;
    }
}
function processFile(target) {
    if (target.files.length == 0) return;
    let img_file = target.files[0];
    var reader = new FileReader();
    reader.onload = e => {
        if( 0 == cur_load_type){
            $("#img1").attr('src', e.target.result);
            img1_data = {
                o_frame: e.target.result
            }
        } else {
            $("#img2").attr('src', e.target.result);
            img2_data = {
                f_frame: e.target.result
            }
        }
    };
    reader.readAsDataURL(img_file);
}
function open_img(type) {
    cur_load_type = type;
    $('input[type="file"]').click();
}
function cmp_face(){
    // sock.emit('speak', '人脸匹配成功')
    $.ajax( {
        type: "POST",
        url: '/get_face_trait',
        // timeout : 3000,
        contentType: "application/json; charset=utf-8",
        data: JSON.stringify({
            img: img1_data.o_frame
        }),
        dataType: "json"
    }) 
    .done( (resp)=> {
        console.log('success', resp );
    })
    .fail( (err)=> { 
        console.log( 'failed: ', err );
    })
}