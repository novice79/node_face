#include "common.h"
#include "tts.h"

void AsyncJob::Execute()
{
    ret_ = Speaker::get().speak(words_);
}
Napi::Object Speaker::Init(Napi::Env env, Napi::Object exports)
{
    exports.Set(
        Napi::String::New(env, "speak"), 
        Napi::Function::New(env, [](const Napi::CallbackInfo& info)->Napi::Value
        {
            Napi::Env env = info.Env();
            if (info.Length() < 2)
            {
                Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
                return env.Null();
            }
            if ( !info[0].IsString() )
            {
                Napi::TypeError::New(env, "Wrong arguments").ThrowAsJavaScriptException();
                return env.Null();
            }
            std::string words = info[0].ToString();
            Napi::Function callback = info[1].As<Napi::Function>();
            AsyncJob* speakWorker = new AsyncJob(callback, words);
            speakWorker->Queue();
            return info.Env().Undefined();
        })
    );
    return exports;
}
Speaker::Speaker()
    :pVoice_(NULL)
{
    if (FAILED(::CoInitialize(NULL)))
        return;
    HRESULT hr = CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, (void **)&pVoice_);
    if( !SUCCEEDED( hr ) )
    {
        pVoice_ = NULL;
        FREEGO_WARN <<"TTS initialize failed\n";
    } else {
        FREEGO_INFO <<"TTS initialize success\n";
    }   
}
Speaker::~Speaker()
    {
        if(pVoice_)
        {
            pVoice_->Release();
            pVoice_ = NULL;
        }
        ::CoUninitialize();
    }
int Speaker::speak(const std::string& words)
{
    if(pVoice_)
    {
        boost::locale::generator gen;
        auto CN = gen.generate("zh_CN.GBK");
        std::stringstream ss;
        ss.imbue(CN);
        ss << boost::locale::conv::from_utf(words, "GBK", boost::locale::conv::skip);
        HRESULT hr = pVoice_->Speak(s2ws(ss.str()).c_str(), 0, NULL);
        if( SUCCEEDED( hr ) ) return 0; else return -1;
    } 
    else 
    {
        FREEGO_WARN <<"TTS initialize failed, can not speak\n";
        return -1;
    }
}