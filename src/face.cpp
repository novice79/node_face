#include "common.h"
#include "face.h"


using namespace std;

FaceTrait::~FaceTrait()
{
    FREEGO_INFO << "FaceTrait::~FaceTrait()\n";
}
void FaceTrait::OnOK()
{
    finish_task();
}
void FaceTrait::OnError(const Napi::Error &e)
{
    Napi::HandleScope scope(Env());
    pt::ptree root;
    stringstream ss;
    root.put("ret", -1);
    root.put("msg", e.Message());
    pt::write_json(ss, root);
    Callback().Call({Napi::String::New(Env(), ss.str())});
}
float FaceTrait::face_diff(const std::string& t1, const std::string& t2)
{
    float diff = -1;
    istringstream sst1(t1), sst2(t2);
    matrix<float,0,1> mat1, mat2;
    try{
        deserialize(mat1, sst1);
        deserialize(mat2, sst2);
        diff = length(mat1 - mat2);
    }catch(...)
    {
        FREEGO_INFO << "人脸特征不合法"<<endl;
    }
    FREEGO_INFO << "人脸差异度："<< diff <<endl;
    return diff;
}
void FaceTrait::cmp_face_traits(const std::string& t1, const std::string& t2)
{
    pt::ptree root;
    stringstream ss;
    float diff = face_diff(t1, t2);
    if(diff < 0)
    {
        root.put("ret", -1);
        root.put("msg", "人脸特征不合法");
        pt::write_json(ss, root);
        auto rdata = ss.str();
        finish_task = [this, rdata]() {
            Napi::HandleScope scope(Env());
            Callback().Call({Napi::String::New(Env(), rdata)});
        };
        return;
    }
    root.put("ret", 0);
    root.put("diff", diff);
    pt::write_json(ss, root);
    auto rdata = ss.str();
    finish_task = [this, rdata]() {
        Napi::HandleScope scope(Env());
        Callback().Call({ Napi::String::New(Env(), rdata) });
    };
}
void FaceTrait::get_face_trait(PImgData imData)
{
    pt::ptree root;
    stringstream ss;
    auto ret = trait_from_image(*imData);
    if(std::get<2>(ret) != 0)
    {
        root.put("ret", -1);
        root.put("msg", "非法的图片");
        pt::write_json(ss, root);
        auto rdata = ss.str();
        finish_task = [this, rdata]() {
            Napi::HandleScope scope(Env());
            Callback().Call({Napi::String::New(Env(), rdata)});
        };
        return;
    }
    auto count = std::get<0>(ret);
    auto trait = std::get<1>(ret);
    if(count != 1)
    {
        root.put("ret", -1);
        root.put("count", count);
        root.put("msg", count == 0 ? "未找到人脸" : "检测到多个人脸，不能获取特征值");
        pt::write_json(ss, root);
        auto rdata = ss.str();
        finish_task = [this, rdata]() {
            Napi::HandleScope scope(Env());
            Callback().Call({Napi::String::New(Env(), rdata)});
        };
        return;
    }
    root.put("ret", 0);
    root.put("count", count);
    pt::write_json(ss, root);
    auto rdata = ss.str();
    finish_task = [this, rdata, trait]() {
        Napi::HandleScope scope(Env());
        Callback().Call({
            Napi::String::New(Env(), rdata), 
            Napi::Buffer<uint8_t>::New( Env(), (uint8_t *)trait->c_str(), trait->size(), FaceTrait::trait_buffer_del_cb, trait) 
        });
    };
}
std::tuple<int, string*, int> FaceTrait::trait_from_image(const std::vector<uint8_t> &imData)
{
    auto sp5 = s_sp5;
    auto res_net = s_res_net;
    std::string *trait = 0;
    cv::Mat o_img, img, im_small;
    double scale_ratio;
    try{        
        // o_img = imdecode(cv::Mat(imData), cv::IMREAD_COLOR); //exception?
        // FREEGO_DEBUG << "image's size = " << o_img.cols << " X " << o_img.rows << endl;
        // 限制在400万像素以内？
        // scale_ratio = o_img.rows > 2000 ? o_img.rows / 2000.0 : 1.0;
        // cv::resize(o_img, img, cv::Size(), 1.0 / scale_ratio, 1.0 / scale_ratio);
        // FREEGO_DEBUG << "readjust image's size = " << img.cols << " X " << img.rows << endl;
        img = imdecode(cv::Mat(imData), cv::IMREAD_COLOR); //exception?
        FREEGO_DEBUG << "image's size = " << img.cols << " X " << img.rows << endl;
        scale_ratio = img.rows > 150 ? img.rows / 150.0 : 1.0;
        cv::resize(img, im_small, cv::Size(), 1.0 / scale_ratio, 1.0 / scale_ratio);
        FREEGO_DEBUG << "im_small's size = " << im_small.cols << " X " << im_small.rows << endl;
    } catch(...){
        return std::make_tuple(0, trait, -1);
    }
    // cv::imwrite("333.jpg", img);
    cv_image<bgr_pixel> cimg_small(im_small);
    cv_image<bgr_pixel> cimg(img);
    auto detector = get_frontal_face_detector();
    std::vector<matrix<rgb_pixel>> faces;
    for (auto face : detector(cimg_small))
    {
        rectangle r(
            (long)(face.left() * scale_ratio),
            (long)(face.top() * scale_ratio),
            (long)(face.right() * scale_ratio),
            (long)(face.bottom() * scale_ratio));
        auto shape = sp5(cimg, r);
        matrix<rgb_pixel> face_chip;
        extract_image_chip(cimg, get_face_chip_details(shape, 150, 0.25), face_chip);
        faces.push_back(move(face_chip));
    }
    // for (auto face : detector(cimg))
    // {
    //     auto shape = sp5(cimg, face);
    //     matrix<rgb_pixel> face_chip;
    //     extract_image_chip(cimg, get_face_chip_details(shape,150,0.25), face_chip);
    //     faces.push_back(move(face_chip));
    // }
    int face_cnt = faces.size();
    if (1 == face_cnt)
    {
        // This call asks the DNN to convert each face image in faces into a 128D vector.
        // In this 128D vector space, images from the same person will be close to each other
        // but vectors from different people will be far apart.  So we can use these vectors to
        // identify if a pair of images are from the same person or from different people.
        std::vector<matrix<float, 0, 1>> face_descriptors = res_net(faces);
        stringstream traits;
        serialize(face_descriptors[0], traits);
        trait = new std::string();
        *trait = traits.str();
    }
    return std::make_tuple(face_cnt, trait, 0);
}
void FaceTrait::Execute()
{
    do_task();
}
anet_type FaceTrait::s_res_net;
shape_predictor FaceTrait::s_sp5;
Napi::Object FaceTrait::Init(Napi::Env env, Napi::Object exports)
{
    deserialize("net.dat") >> s_res_net;
    deserialize("lm5.dat") >> s_sp5;
    exports.Set(
        Napi::String::New(env, "get_face_trait"),
        Napi::Function::New(env, [](const Napi::CallbackInfo &info) -> Napi::Value {
            Napi::Env env = info.Env();
            if (info.Length() < 2)
            {
                Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
                return env.Null();
            }
            if (!info[0].IsBuffer() || !info[1].IsFunction())
            {
                Napi::TypeError::New(env, "Wrong arguments").ThrowAsJavaScriptException();
                return env.Null();
            }

            auto buff = info[0].As<Napi::Buffer<uint8_t>>();
            Napi::Function callback = info[1].As<Napi::Function>();
            auto im_data = std::make_shared<std::vector<uint8_t>>(buff.Data(), buff.Data() + buff.Length());
            FaceTrait *ftWorker = new FaceTrait(callback);
            ftWorker->do_task = std::bind(&FaceTrait::get_face_trait, ftWorker, im_data);
            ftWorker->Queue();
            return info.Env().Undefined();
        }));
    exports.Set(
        Napi::String::New(env, "cmp_traits"),
        Napi::Function::New(env, [](const Napi::CallbackInfo &info) -> Napi::Value {
            Napi::Env env = info.Env();
            if (info.Length() < 3)
            {
                Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
                return env.Null();
            }
            if (! (info[0].IsBuffer() && info[1].IsBuffer() && info[2].IsFunction() ) )
            {
                Napi::TypeError::New(env, "Wrong arguments").ThrowAsJavaScriptException();
                return env.Null();
            }
            auto t1_buff = info[0].As<Napi::Buffer<uint8_t>>();
            auto t2_buff = info[1].As<Napi::Buffer<uint8_t>>();
            Napi::Function callback = info[2].As<Napi::Function>();
            FaceTrait *ftWorker = new FaceTrait(callback);
            ftWorker->do_task = std::bind(
                &FaceTrait::cmp_face_traits, 
                ftWorker, 
                string((char*)t1_buff.Data(), t1_buff.Length()),
                string((char*)t2_buff.Data(), t2_buff.Length())
            );
            ftWorker->Queue();
            return info.Env().Undefined();
        }));    
        export_cmp_images(env, exports);
        export_cmp_trait_and_img(env, exports);
    return exports;
}
Napi::Object FaceTrait::export_cmp_images(Napi::Env env, Napi::Object exports)
{
    exports.Set(
        Napi::String::New(env, "cmp_images"),
        Napi::Function::New(env, [](const Napi::CallbackInfo &info) -> Napi::Value {
            Napi::Env env = info.Env();
            if (info.Length() < 3)
            {
                Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
                return env.Null();
            }
            if (! (info[0].IsBuffer() && info[1].IsBuffer() && info[2].IsFunction() ) )
            {
                Napi::TypeError::New(env, "Wrong arguments").ThrowAsJavaScriptException();
                return env.Null();
            }
            auto img1_buff = info[0].As<Napi::Buffer<uint8_t>>();
            auto img2_buff = info[1].As<Napi::Buffer<uint8_t>>();
            auto img1_data = std::make_shared<std::vector<uint8_t>>(img1_buff.Data(), img1_buff.Data() + img1_buff.Length());
            auto img2_data = std::make_shared<std::vector<uint8_t>>(img2_buff.Data(), img2_buff.Data() + img2_buff.Length());
            Napi::Function callback = info[2].As<Napi::Function>();
            FaceTrait *ftWorker = new FaceTrait(callback);
            ftWorker->do_task = std::bind(
                &FaceTrait::cmp_images, 
                ftWorker, 
                img1_data,
                img2_data
            );
            ftWorker->Queue();
            return info.Env().Undefined();
        }));    
    return exports;
}
Napi::Object FaceTrait::export_cmp_trait_and_img(Napi::Env env, Napi::Object exports)
{
    exports.Set(
        Napi::String::New(env, "cmp_trait_and_img"),
        Napi::Function::New(env, [](const Napi::CallbackInfo &info) -> Napi::Value {
            Napi::Env env = info.Env();
            if (info.Length() < 3)
            {
                Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
                return env.Null();
            }
            if (! (info[0].IsBuffer() && info[1].IsBuffer() && info[2].IsFunction() ) )
            {
                Napi::TypeError::New(env, "Wrong arguments").ThrowAsJavaScriptException();
                return env.Null();
            }
            auto trait_buff = info[0].As<Napi::Buffer<uint8_t>>();
            auto img_buff = info[1].As<Napi::Buffer<uint8_t>>();
            auto img_data = std::make_shared<std::vector<uint8_t>>(img_buff.Data(), img_buff.Data() + img_buff.Length());
            Napi::Function callback = info[2].As<Napi::Function>();
            FaceTrait *ftWorker = new FaceTrait(callback);
            ftWorker->do_task = std::bind(
                &FaceTrait::cmp_trait_and_img, 
                ftWorker, 
                string((char*)trait_buff.Data(), trait_buff.Length()),
                img_data
            );
            ftWorker->Queue();
            return info.Env().Undefined();
        }));    
    return exports;
}
void FaceTrait::cmp_images(PImgData img1, PImgData img2)
{
    pt::ptree root;
    stringstream ss;
    auto dd1 = std::async(&FaceTrait::trait_from_image, this, *img1);
	auto dd2 = std::async(&FaceTrait::trait_from_image, this, *img2);
    auto ret1 = dd1.get();
    auto ret2 = dd2.get();
    if(std::get<2>(ret1) != 0 || std::get<2>(ret2) != 0)
    {
        root.put("ret", -1);
        root.put("msg", "invalid image");
        pt::write_json(ss, root);
        auto rdata = ss.str();
        finish_task = [this, rdata]() {
            Napi::HandleScope scope(Env());
            Callback().Call({Napi::String::New(Env(), rdata)});
        };
        return;
    }
    // auto ret = trait_from_image(*imData);
    auto count1 = std::get<0>(ret1);
    auto count2 = std::get<0>(ret2);
    if(count1 != 1 || count2 != 1)
    {
        root.put("ret", -1);
        root.put("count1", count1);
        root.put("count2", count2);
        pt::write_json(ss, root);
        auto rdata = ss.str();
        finish_task = [this, rdata]() {
            Napi::HandleScope scope(Env());
            Callback().Call({Napi::String::New(Env(), rdata)});
        };
        return;
    }
    auto ptrait1 = std::get<1>(ret1);
    auto ptrait2 = std::get<1>(ret2);
    float diff = face_diff(*ptrait1, *ptrait2);
    root.put("ret", 0);
    root.put("diff", diff);
    pt::write_json(ss, root);
    auto rdata = ss.str();
    FREEGO_INFO << "in FaceTrait::cmp_images: "<< rdata <<endl;
    finish_task = [this, rdata, ptrait1, ptrait2]() {
        Napi::HandleScope scope(Env());
        Callback().Call({
            Napi::String::New(Env(), rdata),
            Napi::Buffer<uint8_t>::New(Env(), (uint8_t *)ptrait1->c_str(), ptrait1->size(), FaceTrait::trait_buffer_del_cb, ptrait1),
            Napi::Buffer<uint8_t>::New(Env(), (uint8_t *)ptrait2->c_str(), ptrait2->size(), FaceTrait::trait_buffer_del_cb, ptrait2)
        });
    };
}
void FaceTrait::cmp_trait_and_img(const std::string& trait, PImgData img)
{
    pt::ptree root;
    stringstream ss;
    auto dd = std::async(&FaceTrait::trait_from_image, this, *img);
    auto ret = dd.get();    
    if(std::get<2>(ret) != 0)
    {
        root.put("ret", -1);
        root.put("msg", "invalid image");
        pt::write_json(ss, root);
        auto rdata = ss.str();
        finish_task = [this, rdata]() {
            Napi::HandleScope scope(Env());
            Callback().Call({Napi::String::New(Env(), rdata)});
        };
        return;
    }
    auto count = std::get<0>(ret);
    if(count != 1)
    {
        root.put("ret", -1);
        root.put("count", count);
        pt::write_json(ss, root);
        auto rdata = ss.str();
        finish_task = [this, rdata]() {
            Napi::HandleScope scope(Env());
            Callback().Call({Napi::String::New(Env(), rdata)});
        };
        return;
    }
    string* ptrait = std::get<1>(ret);
    float diff = face_diff(*ptrait, trait);
    root.put("ret", 0);
    root.put("diff", diff);
    pt::write_json(ss, root);
    auto rdata = ss.str();
    finish_task = [this, rdata, &ptrait]() {
        Napi::HandleScope scope(Env());
        Callback().Call({
            Napi::String::New(Env(), rdata),
            Napi::Buffer<uint8_t>::New(Env(), (uint8_t *)ptrait->c_str(), ptrait->size(), FaceTrait::trait_buffer_del_cb, ptrait)
        });
    };
}
// ----------------------------------------------------------------------------------------
std::vector<matrix<rgb_pixel>> jitter_image(
    const matrix<rgb_pixel> &img)
{
    // All this function does is make 100 copies of img, all slightly jittered by being
    // zoomed, rotated, and translated a little bit differently. They are also randomly
    // mirrored left to right.
    thread_local dlib::rand rnd;

    std::vector<matrix<rgb_pixel>> crops;
    for (int i = 0; i < 100; ++i)
        crops.push_back(jitter_image(img, rnd));

    return crops;
}
// ----------------------------------------------------------------------------------------