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
    Callback().Call({Napi::String::New(Env(), "somethings wrong")});
}
void FaceTrait::cmp_face_traits(const std::string& t1, const std::string& t2)
{
    istringstream sst1(t1), sst2(t2);
    matrix<float,0,1> mat1, mat2;
    deserialize(mat1, sst1);
    deserialize(mat2, sst2);
    float diff = length(mat1 - mat2);
    FREEGO_INFO << "人脸差异度："<< diff <<endl;
    finish_task = [this, diff]() {
        Napi::HandleScope scope(Env());
        Callback().Call({ Env().Undefined(),  Napi::Number::New(Env(), diff) });
    };
}
void FaceTrait::get_face_trait(PImgData imData)
{
    auto ret = trait_from_image(*imData);
    auto count = std::get<0>(ret);
    auto trait = std::get<1>(ret);
    finish_task = [this, count, trait]() {
        Napi::HandleScope scope(Env());
        Callback().Call({Env().Undefined(),
                         Napi::Number::New(Env(), count),
                         trait ? Napi::Buffer<uchar>::New(
                                      Env(), (uchar *)trait->c_str(), trait->size(), FaceTrait::trait_buffer_del_cb, trait)
                                : Env().Null()});
    };
}
std::tuple<int, string*> FaceTrait::trait_from_image(const std::vector<uchar> &imData)
{
    std::string *trait = 0;
    cv::Mat img = imdecode(cv::Mat(imData), cv::IMREAD_COLOR), im_small;
    cv::resize(img, im_small, cv::Size(), 1.0 / FACE_DOWNSAMPLE_RATIO, 1.0 / FACE_DOWNSAMPLE_RATIO);
    FREEGO_DEBUG << "image's size = " << img.cols << " X " << img.rows << endl;
    // cv::imwrite("333.jpg", img);
    cv_image<bgr_pixel> cimg_small(im_small);
    cv_image<bgr_pixel> cimg(img);
    auto detector = get_frontal_face_detector();
    std::vector<matrix<rgb_pixel>> faces;
    // for (auto face : detector(cimg_small))
    // {
    //     rectangle r(
    //         (long)(face.left() * FACE_DOWNSAMPLE_RATIO),
    //         (long)(face.top() * FACE_DOWNSAMPLE_RATIO),
    //         (long)(face.right() * FACE_DOWNSAMPLE_RATIO),
    //         (long)(face.bottom() * FACE_DOWNSAMPLE_RATIO));
    //     auto shape = s_sp5(cimg, r);
    //     matrix<rgb_pixel> face_chip;
    //     extract_image_chip(cimg, get_face_chip_details(shape, 150, 0.25), face_chip);
    //     faces.push_back(move(face_chip));
    // }
    for (auto face : detector(cimg))
    {
        auto shape = s_sp5(cimg, face);
        matrix<rgb_pixel> face_chip;
        extract_image_chip(cimg, get_face_chip_details(shape,150,0.25), face_chip);
        faces.push_back(move(face_chip));
    }
    int face_cnt = faces.size();
    if (1 == face_cnt)
    {
        // This call asks the DNN to convert each face image in faces into a 128D vector.
        // In this 128D vector space, images from the same person will be close to each other
        // but vectors from different people will be far apart.  So we can use these vectors to
        // identify if a pair of images are from the same person or from different people.
        std::vector<matrix<float, 0, 1>> face_descriptors = s_res_net(faces);
        stringstream traits;
        serialize(face_descriptors[0], traits);
        trait = new std::string();
        *trait = traits.str();
    }
    return std::make_tuple(face_cnt, trait);
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

            auto buff = info[0].As<Napi::Buffer<uchar>>();
            Napi::Function callback = info[1].As<Napi::Function>();
            auto im_data = std::make_shared<std::vector<uchar>>(buff.Data(), buff.Data() + buff.Length());
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
            auto t1_buff = info[0].As<Napi::Buffer<uchar>>();
            auto t2_buff = info[1].As<Napi::Buffer<uchar>>();
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
    return exports;
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