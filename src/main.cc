#include "common.h"
#include "handle_map.h"

namespace {

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  CommonInit(env);
  PlatformInit();

  exports["setCallback"] = Napi::Function::New(env, &SetCallback);
  exports["watch"] = Napi::Function::New(env, &Watch);
  exports["unwatch"] = Napi::Function::New(env, &Unwatch);

  HandleMap::Initialize(env, exports);
  return exports;
}

}  // namespace

NODE_API_MODULE(pathwatcher, Init)
