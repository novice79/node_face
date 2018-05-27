#include "worker.h"

using namespace std;
Napi::FunctionReference NativeWorker::constructor;
Napi::Object NativeWorker::Init(Napi::Env env, Napi::Object exports)
{
  Napi::HandleScope scope(env);
  Napi::Function func = DefineClass(
      env,
      "NativeWorker",
      {
          InstanceMethod("on", &NativeWorker::On),
          InstanceMethod("emit", &NativeWorker::Emit),
      });
  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();
  exports.Set("NativeWorker", func);
  return exports;
}

NativeWorker::NativeWorker(const Napi::CallbackInfo &info)
    : Napi::ObjectWrap<NativeWorker>(info)
{
  uv_async_init(uv_default_loop(), &handle_, &static_async_callback);
  handle_.data = this;
  run();
}
void NativeWorker::stage0()
{
  int i = 99;
  
  send([this, i](){
    auto it = evt2cb_.find("stage0");
    if (it != evt2cb_.end())
    {
      // cout<<"found  stage0 event, result: "<<i <<endl;
      auto pcb = it->second;
      auto env = pcb->env();
      Napi::HandleScope scope(env);
      std::vector<napi_value> args = { 
        Napi::Number::New(env, i)
      };     
      (*pcb)(args);
    }   
  });
}
void NativeWorker::run()
{
  std::thread t([=]() {
    while (1)
    {
      stage0();
      std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    }
  });
  t.detach();
}
Napi::Value NativeWorker::Emit(const Napi::CallbackInfo &info)
{
  double num = 0;

  return Napi::Number::New(info.Env(), num);
}

Napi::Value NativeWorker::On(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  if (info.Length() < 2)
  {
    Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
    return env.Null();
  }
  if (!info[0].IsString() || !info[1].IsFunction())
  {
    Napi::TypeError::New(env, "Wrong arguments").ThrowAsJavaScriptException();
    return env.Null();
  }
  string evt_name = info[0].ToString();
  Napi::Function cb = info[1].As<Napi::Function>();

  evt2cb_[evt_name] = std::make_shared<Subscriber>(std::move(Napi::Persistent(cb)));

  return Napi::Number::New(info.Env(), 0);
}
void NativeWorker::async_callback()
{
  while (true)
  {
    std::vector<Messager> msgers;
    {
      std::lock_guard<std::mutex> lock(mutex_);
      if (messagers_.empty())
        break;
      else
        msgers.swap(messagers_);
    }
    for (const auto &m : msgers)
    {
      m();
    }
  }

  // if (close_)
  //   uv_close(reinterpret_cast<uv_handle_t *>(&handle_), [](uv_handle_t *handle) {
  //     delete static_cast<Impl *>(handle->data);
  //   });
}