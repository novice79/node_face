#include "common.h"

#include "face.h"
#include "worker.h"


Napi::Object Init(Napi::Env env, Napi::Object exports) {
  NativeWorker::Init(env, exports);
  FaceTrait::Init(env, exports);
  return exports;
}

NODE_API_MODULE(addon, Init)
