
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
                o_frame: e.target.result,
                f_frame: e.target.result
            }
        } else {
            $("#img2").attr('src', e.target.result);
            img2_data = {
                o_frame: e.target.result,
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
function speak_input(){
    const words = $("#words").val();
    if(!words) return alert('请先输入文字')
    speak( words );
}
function cmp_face(){
    // sock.emit('speak', '人脸匹配成功')
    if(img1_data.count && img1_data.count != 1){
        return sock.emit('speak', '左侧人脸个数不唯一')
    }
    if(img2_data.count && img2_data.count != 1){
        return sock.emit('speak', '右侧人脸个数不唯一')
    }
    if(img1_data.trait && img2_data.trait){
        return cmp_face_by_traits(img1_data.trait, img2_data.trait, (err, data)=>{
            if(err){
                sock.emit('speak', '人脸比较失败')
            } else {
                if(data.ret == -1) return sock.emit('speak', data.msg);
                //相差度小于0.5即认为是同一个人
                if(data.diff < 0.5){
                    sock.emit('speak', '是同一人')
                } else {
                    sock.emit('speak', '不是同一人')
                }
            }
        })
    }
    if(img1_data.trait || img2_data.trait){
        let trait, img, to_be_filled;
        if(img1_data.trait){
            trait = img1_data.trait;
            img = img2_data.o_frame;
            to_be_filled = img2_data;
        } else {
            trait = img2_data.trait;
            img = img1_data.o_frame;
            to_be_filled = img1_data;
        }
        return cmp_face_by_trait_and_img(trait, img, (err, data)=>{
            if(err){
                sock.emit('speak', '人脸比较失败')
            } else {
                if(data.ret == 0){
                    if(data.diff < 0.5){
                        sock.emit('speak', '是同一人')
                    } else {
                        sock.emit('speak', '不是同一人')
                    }
                    to_be_filled.count = 1;
                    to_be_filled.trait = data.trait;
                } else {
                    to_be_filled.count = data.count;
                    sock.emit('speak', '照片包含多人，不能比较')
                }                
            }
        })
    }
    cmp_face_by_imgs(img1_data.o_frame, img2_data.o_frame, (err, data)=>{
        if(err){
            sock.emit('speak', '人脸比较失败')
        } else {
            if(data.ret == 0){
                if(data.diff < 0.5){
                    sock.emit('speak', '是同一人')
                } else {
                    sock.emit('speak', '不是同一人')
                }
                img1_data.count = img2_data.count = 1;
                img1_data.trait = data.trait1;
                img2_data.trait = data.trait2;
            } else {
                img1_data.count = data.count1;
                img2_data.count = data.count2;
                sock.emit('speak', '照片包含多人，不能比较')
            }                
        }
    })
}