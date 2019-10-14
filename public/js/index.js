
let img1_data, img2_data, cur_load_type;


function processFile(target) {
    if (target.files.length == 0) return;
    let img_file = target.files[0];
    let reader = new FileReader();
    reader.onload = e => {
        if( 0 == cur_load_type){
            $("#img1").attr('src', e.target.result);
            img1_data = {
                o_frame: e.target.result
            };
        } else {
            $("#img2").attr('src', e.target.result);
            img2_data = {
                o_frame: e.target.result
            };
        }
    };
    reader.readAsDataURL(img_file);
}
function open_img(type) {
    cur_load_type = type;
    $('input[type="file"]').click();
}
function get_trait(which){
    if( !( (img1_data && img1_data.o_frame) || (img2_data && img2_data.o_frame)) ) return alert('未选择图片');
    let img = which == 0 ? img1_data.o_frame : img2_data.o_frame;
    get_face_trait(img, function(err, res){
        if(err){
            alert('获取特征失败');
        } else{
            if(res.ret != 0){
                alert('获取特征失败:'+res.msg);
            } else{
                if(which == 0){
                    img1_data.trait = res.trait;
                } else {
                    img2_data.trait = res.trait;
                }
                alert('获取特征成功:'+res.trait);
            }
        }
    });
  
}
function cmp_face(){

    if(img1_data.count && img1_data.count != 1){
        alert('左侧人脸个数不唯一')
    }
    if(img2_data.count && img2_data.count != 1){
        alert('右侧人脸个数不唯一')
    }
    if(img1_data.trait && img2_data.trait){
        return cmp_face_by_traits(img1_data.trait, img2_data.trait, (err, data)=>{
            if(err){
                alert('人脸比较失败')
            } else {
                if(data.ret == -1) return alert(data.msg);
                //相差度小于0.5即认为是同一个人
                if(data.diff < 0.5){
                    alert('是同一人')
                } else {
                    alert('不是同一人')
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
                alert('人脸比较失败')
            } else {
                if(data.ret == 0){
                    if(data.diff < 0.5){
                        alert('是同一人')
                    } else {
                        alert('不是同一人')
                    }
                    to_be_filled.count = 1;
                    to_be_filled.trait = data.trait;
                } else {
                    to_be_filled.count = data.count;
                    alert('照片包含多人，不能比较')
                }                
            }
        })
    }
    cmp_face_by_imgs(img1_data.o_frame, img2_data.o_frame, (err, data)=>{
        if(err){
            alert('人脸比较失败')
        } else {
            if(data.ret == 0){
                if(data.diff < 0.5){
                    alert('是同一人')
                } else {
                    alert('不是同一人')
                }
                img1_data.count = img2_data.count = 1;
                img1_data.trait = data.trait1;
                img2_data.trait = data.trait2;
            } else {
                img1_data.count = data.count1;
                img2_data.count = data.count2;
                alert('照片包含0人或多人，不能比较')
            }                
        }
    })
}