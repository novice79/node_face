#include "stdafx.h"
#include <napi.h>

#include "export.h"

void RunCallback(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::Function cb = info[0].As<Napi::Function>();
  cb.MakeCallback(env.Global(), { Napi::String::New( env, test_cpp().c_str() ) });
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  return Napi::Function::New(env, RunCallback);
}

NODE_API_MODULE(addon, Init)
