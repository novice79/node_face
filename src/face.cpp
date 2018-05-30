#include "common.h"
#include "face.h"

extern shape_predictor g_sp;
extern anet_type g_net;
using namespace std;

FaceTrait::~FaceTrait() {
    FREEGO_INFO <<"FaceTrait::~FaceTrait()\n";
}
void FaceTrait::OnOK()
{
    Napi::HandleScope scope(Env());
    // FREEGO_DEBUG <<"7777777777777777777777777\n";
    Callback().Call({
        Env().Undefined(), 
        Napi::Number::New(Env(), count_), 
        trait_ ?
        Napi::Buffer<uchar>::New(Env(), (uchar*)trait_->c_str(), trait_->size(), FaceTrait::trait_buffer_del_cb, trait_ )
        : Env().Null()
    });
}
void FaceTrait::OnError (const Napi::Error &e)
{
    Napi::HandleScope scope(Env());
    Callback().Call({Napi::String::New(Env(), "somethings wrong")});
}
void FaceTrait::Execute()
{    
    cv::Mat img = imdecode(cv::Mat(*imData_), cv::IMREAD_COLOR );
    FREEGO_DEBUG <<"image's size = " << img.cols << " X " << img.rows << endl;
    // cv::imwrite("333.jpg", img);
    cv_image<bgr_pixel> cimg(img);
    // FREEGO_DEBUG <<"00000000000000000000000000\n";
    auto detector = get_frontal_face_detector();
    std::vector<matrix<rgb_pixel>> faces;
    for (auto face : detector(cimg))
    {
        // FREEGO_DEBUG <<"44444444444444444444444\n";
        auto shape = g_sp(cimg, face);
        matrix<rgb_pixel> face_chip;
        extract_image_chip(cimg, get_face_chip_details(shape,150,0.25), face_chip);
        faces.push_back(move(face_chip));
    }
    count_ = faces.size();
    // FREEGO_DEBUG <<"55555555555555555555555555\n";
    if( 1 == count_ )
    {
        // This call asks the DNN to convert each face image in faces into a 128D vector.
        // In this 128D vector space, images from the same person will be close to each other
        // but vectors from different people will be far apart.  So we can use these vectors to
        // identify if a pair of images are from the same person or from different people.  
        std::vector<matrix<float,0,1>> face_descriptors = g_net(faces);
        stringstream traits;
        serialize(face_descriptors[0], traits);
        trait_ = new std::string();
        *trait_ = traits.str();
        // FREEGO_DEBUG <<"66666666666666666666666\n";
    }
}
Napi::Object FaceTrait::Init(Napi::Env env, Napi::Object exports)
{
    exports.Set(
        Napi::String::New(env, "get_face_trait"), 
        Napi::Function::New(env, [](const Napi::CallbackInfo& info)->Napi::Value
        {
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
            // FREEGO_DEBUG <<"1111111111111111111111111111\n";
            auto buff = info[0].As< Napi::Buffer<uchar> >();
            Napi::Function callback = info[1].As<Napi::Function>();
            auto im_data = std::make_unique<std::vector<uchar>>( buff.Data(), buff.Data() + buff.Length() );
            // FREEGO_DEBUG <<"222222222222222222222222\n";
            FaceTrait* ftWorker = new FaceTrait(callback, std::move(im_data));
            ftWorker->Queue();
            return info.Env().Undefined();
        })
    );
    return exports;
}

// ----------------------------------------------------------------------------------------
std::vector<matrix<rgb_pixel>> jitter_image(
    const matrix<rgb_pixel>& img
)
{
    // All this function does is make 100 copies of img, all slightly jittered by being
    // zoomed, rotated, and translated a little bit differently. They are also randomly
    // mirrored left to right.
    thread_local dlib::rand rnd;

    std::vector<matrix<rgb_pixel>> crops; 
    for (int i = 0; i < 100; ++i)
        crops.push_back(jitter_image(img,rnd));

    return crops;
}
// ----------------------------------------------------------------------------------------