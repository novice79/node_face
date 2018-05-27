#include "stdafx.h"


#include "export.h"
#include "worker.h"

Napi::Value StartVideo(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::Function cb = info[0].As<Napi::Function>();
  int ret = test(cb);
  return Napi::Number::New(info.Env(), ret);
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set(Napi::String::New(env, "startVideo"), Napi::Function::New(env, StartVideo));
  return NativeWorker::Init(env, exports);
  // return exports;
}

NODE_API_MODULE(addon, Init)
