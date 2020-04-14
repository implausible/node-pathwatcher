#include "handle_map.h"

#include <algorithm>

Napi::FunctionReference HandleMap::constructor;

HandleMap::HandleMap(const Napi::CallbackInfo &info)
  : Napi::ObjectWrap<HandleMap>(info) {
}

HandleMap::~HandleMap() {
  Clear();
}

bool HandleMap::Has(WatcherHandle key) const {
  return map_.find(key) != map_.end();
}

bool HandleMap::Erase(WatcherHandle key) {
  Map::iterator iter = map_.find(key);
  if (iter == map_.end())
    return false;

  iter->second.Reset();
  map_.erase(iter);
  return true;
}

void HandleMap::Clear() {
  for (Map::iterator iter = map_.begin(); iter != map_.end(); ++iter)
    iter->second.Reset();
  map_.clear();
}

Napi::Value HandleMap::Add(const Napi::CallbackInfo &info) {
  auto env = info.Env();
  if (!IsV8ValueWatcherHandle(info[0]))
    throw Napi::TypeError::New(env, "Bad argument");

  WatcherHandle key = V8ValueToWatcherHandle(info[0]);
  if (Has(key))
    throw Napi::Error::New(env, "Duplicate key");

  map_[key] = Napi::Reference<Napi::Value>::New(info[1], 1);
  return env.Undefined();
}

Napi::Value HandleMap::Get(const Napi::CallbackInfo &info) {
  auto env = info.Env();
  if (!IsV8ValueWatcherHandle(info[0]))
    throw Napi::TypeError::New(env, "Bad argument");

  WatcherHandle key = V8ValueToWatcherHandle(info[0]);
  if (!Has(key))
    throw Napi::Error::New(env, "Invalid key");

  return map_[key].Value();
}

Napi::Value HandleMap::Has(const Napi::CallbackInfo &info) {
  auto env = info.Env();
  if (!IsV8ValueWatcherHandle(info[0]))
    throw Napi::TypeError::New(env, "Bad argument");

  return Napi::Boolean::New(env, Has(V8ValueToWatcherHandle(info[0])));
}

Napi::Value HandleMap::Values(const Napi::CallbackInfo &info) {
  auto env = info.Env();
  size_t i = 0;
  Napi::Array keys = Napi::Array::New(env, map_.size());
  for (Map::const_iterator iter = map_.begin();
       iter != map_.end();
       ++iter, ++i) {
    keys[i] = iter->second.Value();
  }

  return keys;
}

Napi::Value HandleMap::Remove(const Napi::CallbackInfo &info) {
  auto env = info.Env();
  if (!IsV8ValueWatcherHandle(info[0]))
    throw Napi::TypeError::New(env, "Bad argument");

  if (!Erase(V8ValueToWatcherHandle(info[0])))
    throw Napi::Error::New(env, "Invalid key");

  return env.Undefined();
}

Napi::Value HandleMap::Clear(const Napi::CallbackInfo &info) {
  Clear();
  return info.Env().Undefined();
}

// static
void HandleMap::Initialize(Napi::Env env, Napi::Object exports) {
  Napi::Function handleMapConstructor = DefineClass(env, "HandleMap", {
    InstanceMethod("add", &HandleMap::Add),
    InstanceMethod("get", &HandleMap::Get),
    InstanceMethod("has", &HandleMap::Has),
    InstanceMethod("values", &HandleMap::Values),
    InstanceMethod("remove", &HandleMap::Remove),
    InstanceMethod("clear", &HandleMap::Clear)
  });

  constructor = Napi::Persistent(handleMapConstructor);
  constructor.SuppressDestruct();

  exports["HandleMap"] = handleMapConstructor;
}
