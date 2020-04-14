#ifndef SRC_HANDLE_MAP_H_
#define SRC_HANDLE_MAP_H_

#include <map>

#include "common.h"

class HandleMap : public Napi::ObjectWrap<HandleMap> {
 public:
  static void Initialize(Napi::Env env, Napi::Object exports);
  HandleMap(const Napi::CallbackInfo &info);
  virtual ~HandleMap();

 private:
  static Napi::FunctionReference constructor;
  typedef std::map<WatcherHandle, Napi::Reference<Napi::Value> > Map;

  bool Has(WatcherHandle key) const;
  bool Erase(WatcherHandle key);
  void Clear();

  static void DisposeHandle(Napi::Reference<Napi::Value> &value);

  Napi::Value Add(const Napi::CallbackInfo &info);
  Napi::Value Get(const Napi::CallbackInfo &info);
  Napi::Value Has(const Napi::CallbackInfo &info);
  Napi::Value Values(const Napi::CallbackInfo &info);
  Napi::Value Remove(const Napi::CallbackInfo &info);
  Napi::Value Clear(const Napi::CallbackInfo &info);

  Map map_;
};

#endif  // SRC_HANDLE_MAP_H_
