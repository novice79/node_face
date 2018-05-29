#include "stdafx.h"

#include "face.h"
#include "worker.h"
#include "tts.h"

Napi::Value StartVideo(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::Function cb = info[0].As<Napi::Function>();
  int ret = test(cb);
  return Napi::Number::New(info.Env(), ret);
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set(Napi::String::New(env, "startVideo"), Napi::Function::New(env, StartVideo));
  NativeWorker::Init(env, exports);
  Speaker::Init(env, exports);
  return exports;
}

NODE_API_MODULE(addon, Init)
