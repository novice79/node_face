
#pragma once
#include "def.h"

class FaceTrait : public Napi::AsyncWorker
{
    static void trait_buffer_del_cb(Napi::Env env, uchar *data, std::string *hint)
    {
        // FREEGO_TRACE <<"in buffer_delete_callback, hint="<<*hint<<endl;
        delete hint;
    }
    static anet_type s_res_net;
    static shape_predictor s_sp5;
  public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);

    FaceTrait(Napi::Function &callback)
        : Napi::AsyncWorker(callback) {}
    ~FaceTrait();
    void Execute();
    void OnOK();
    void OnError(const Napi::Error &e);
    //////////////////////////////////
    typedef std::shared_ptr<std::vector<uchar>> PImgData;
    void get_face_trait(PImgData imData);
    void cmp_face_traits(const std::string& t1, const std::string& t2);
    void cmp_images(PImgData img1, PImgData img2);
    void cmp_trait_and_img(const std::string& trait, PImgData img);
    std::function<void()> do_task;
  private:
    std::function<void()> finish_task;
    //return face count && face trait(if count == 1)
    std::tuple<int, std::string*> trait_from_image(const std::vector<uchar> &imData);
    float face_diff(const std::string& t1, const std::string& t2);
    static Napi::Object export_cmp_images(Napi::Env env, Napi::Object exports);
    static Napi::Object export_cmp_trait_and_img(Napi::Env env, Napi::Object exports);
  private:
};

std::vector<matrix<rgb_pixel>> jitter_image(
    const matrix<rgb_pixel> &img);
