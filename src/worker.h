#pragma once
#include <uv.h>
#include <napi.h>
#include <map>

class NativeWorker : public Napi::ObjectWrap<NativeWorker> {
  class Subscriber
  {
    Napi::Reference<Napi::Value> receiver_;
    Napi::FunctionReference      callback_;
    public:
      Subscriber( Napi::FunctionReference &&callback)
      :callback_( std::move(callback) )
      {
        receiver_ = Napi::Persistent(static_cast<Napi::Value>(Napi::Object::New(callback_.Env())));
      }
      void operator()(const std::vector<napi_value>& args)
      {
        callback_.MakeCallback(receiver_.Value(), args);
      }
      Napi::Env env()
      {
        return callback_.Env();
      }
  };
  void run();
  void async_callback();
  static void static_async_callback(uv_async_t *handle)
  {
      try
      {
          static_cast<NativeWorker *>(handle->data)->async_callback();
      }
      catch (std::exception& e)
      {
          Napi::Error::Fatal("", e.what());
      }
      catch (...) 
      {
          Napi::Error::Fatal("", "ERROR: Unknown exception during async callback");
      }
  }
  void send(std::function<void()> m)
  {
      std::lock_guard<std::mutex> lock(mutex_);
      messagers_.push_back({m});
      uv_async_send(&handle_);
  }
  void stage0();
  // void stage1();
  // void stage2();
 public:
  static Napi::Object Init(Napi::Env env, Napi::Object exports);
  NativeWorker(const Napi::CallbackInfo& info);

 private:
  static Napi::FunctionReference constructor;

  Napi::Value On(const Napi::CallbackInfo& info);
  Napi::Value Emit(const Napi::CallbackInfo& info);
  std::map<std::string, std::shared_ptr<Subscriber> > evt2cb_;
  uv_async_t                   handle_;
  std::mutex                   mutex_;
  using Messager = std::function<void()>;
  std::vector<Messager>     messagers_;

};
