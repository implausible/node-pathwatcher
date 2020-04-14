#ifndef SRC_COMMON_H_
#define SRC_COMMON_H_

#include <condition_variable>
#include <mutex>
#include <napi.h>
#include <string>
#include <thread>
#include <uv.h>
#include <vector>

#ifdef _WIN32
// Platform-dependent definetion of handle.
typedef HANDLE WatcherHandle;

// Conversion between V8 value and WatcherHandle.
#define WatcherHandleToV8Value(h) Napi::External<void>::New(env, h)
#define V8ValueToWatcherHandle(v) v.As<Napi::External<void>>().Data()
#define IsV8ValueWatcherHandle(v) v.Type() == napi_external
#else
// Correspoding definetions on OS X and Linux.
typedef int32_t WatcherHandle;
#define WatcherHandleToV8Value(h) Napi::Number::New(env, h)
#define V8ValueToWatcherHandle(v) v.ToNumber().Int32Value()
#define IsV8ValueWatcherHandle(v) v.IsNumber()
#endif

void PlatformInit();
void PlatformThread();
WatcherHandle PlatformWatch(const char* path);
void PlatformUnwatch(WatcherHandle handle);
bool PlatformIsHandleValid(WatcherHandle handle);
int PlatformInvalidHandleToErrorNumber(WatcherHandle handle);

enum EVENT_TYPE {
  EVENT_NONE,
  EVENT_CHANGE,
  EVENT_RENAME,
  EVENT_DELETE,
  EVENT_CHILD_CHANGE,
  EVENT_CHILD_RENAME,
  EVENT_CHILD_DELETE,
  EVENT_CHILD_CREATE,
};

void WaitForPlatformInitialization();
void SignalPlatformInitialized();
void PostEventAndWait(EVENT_TYPE type,
                      WatcherHandle handle,
                      const std::vector<char>& new_path,
                      const std::vector<char>& old_path = std::vector<char>());

void CommonInit(Napi::Env);

Napi::Value SetCallback(const Napi::CallbackInfo &info);
Napi::Value Watch(const Napi::CallbackInfo &info);
Napi::Value Unwatch(const Napi::CallbackInfo &info);

#endif  // SRC_COMMON_H_
