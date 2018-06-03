function download_img(url, name){
    var a = document.createElement('a');
    a.href = url;
    a.download = name;
    document.body.appendChild(a);
    a.click();
    document.body.removeChild(a);
}
function get_img_trait(img, cb){
    let a = new Date().getTime(), b;
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
        cb && cb(null, resp);
        b = new Date().getTime();
        console.log(`/根据图片获取人脸特征，耗时: %d ms`, b-a);
    })
    .fail( (err)=> { 
        console.log( 'failed: ', err );
        cb && cb(err);
    })
}

function cmp_face_by_imgs(img1, img2, cb){
    let a = new Date().getTime(), b;
    $.ajax( {
        type: "POST",
        url: '/cmp_face_by_imgs',
        contentType: "application/json; charset=utf-8",
        data: JSON.stringify({
            img1,
            img2
        }),
        dataType: "json"
    }) 
    .done( (resp)=> {
        console.log('success', resp );
        cb && cb(null, resp);
        b = new Date().getTime();
        console.log(`/根据【图片】比较相似度，耗时: %d ms`, b-a);
    })
    .fail( (err)=> { 
        console.log( 'failed: ', err );
        cb && cb(err);
    })
}
function cmp_face_by_traits(trait1, trait2, cb){
    let a = new Date().getTime(), b;
    $.ajax( {
        type: "POST",
        url: '/cmp_face_by_traits',
        contentType: "application/json; charset=utf-8",
        data: JSON.stringify({
            trait1,
            trait2
        }),
        dataType: "json"
    }) 
    .done( (resp)=> {
        console.log('success', resp );
        cb && cb(null, resp);
        b = new Date().getTime();
        console.log(`/根据【人脸特征】比较相似度，耗时: %d ms`, b-a);
    })
    .fail( (err)=> { 
        console.log( 'failed: ', err );
        cb && cb(err);
    })
}
function cmp_face_by_trait_and_img(trait, img, cb){
    let a = new Date().getTime(), b;
    $.ajax( {
        type: "POST",
        url: '/cmp_face_by_trait_and_img',
        contentType: "application/json; charset=utf-8",
        data: JSON.stringify({
            trait,
            img
        }),
        dataType: "json"
    }) 
    .done( (resp)=> {
        console.log('success', resp );
        cb && cb(null, resp);
        b = new Date().getTime();
        console.log(`/根据人脸【特征和图片】比较相似度，耗时: %d ms`, b-a);
    })
    .fail( (err)=> { 
        console.log( 'failed: ', err );
        cb && cb(err);
    })
}