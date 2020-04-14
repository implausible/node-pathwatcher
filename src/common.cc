#include "common.h"

class SingleUseSemaphore {
  public:
    SingleUseSemaphore()
      : m_signaled(false) {}

    void wait() {
      std::unique_lock<std::mutex> lock(m_mutex);

      while (!m_signaled) m_cond.wait(lock);
    }

    void signal() {
      std::unique_lock<std::mutex> lock(m_mutex);
      m_signaled = true;
      m_cond.notify_all();
    }

  private:
    std::mutex m_mutex;
    std::condition_variable m_cond;
    bool m_signaled;
};

static SingleUseSemaphore g_semaphore;
static Napi::FunctionReference g_actual_callback;
static Napi::ThreadSafeFunction g_callback;

struct EventPayload {
  EventPayload(EVENT_TYPE p_type, WatcherHandle p_handle, const std::vector<char> &p_new_path, const std::vector<char> &p_old_path)
    : type(p_type), handle(p_handle), new_path(p_new_path), old_path(p_old_path) {}
  EVENT_TYPE type;
  WatcherHandle handle;
  const std::vector<char> &new_path,
                          &old_path;
};

static void CommonThread(void* handle) {
  WaitForPlatformInitialization();
  PlatformThread();
}

void CommonInit(Napi::Env env) {
  g_callback = Napi::ThreadSafeFunction::New(
    env,
    Napi::Function::New(env, [](const Napi::CallbackInfo &info) {
      // Empty, because we're not ever going to use it due to the setter pattern (setCallback)
      // It would be pretty difficult to resynchronize the threadsafe function across threads anytime
      // setCallback is called.
    }),
    "pathWatcher",
    0,
    3);
  g_callback.Unref(env);
  std::thread(&CommonThread, nullptr).detach();
}

void WaitForPlatformInitialization() {
  g_semaphore.wait();
}

void SignalPlatformInitialized() {
  g_semaphore.signal();
}

void PostEventAndWait(EVENT_TYPE type,
                      WatcherHandle handle,
                      const std::vector<char> &new_path,
                      const std::vector<char> &old_path) {
  g_callback.BlockingCall(
    new EventPayload(type, handle, new_path, old_path),
    [](Napi::Env env, Napi::Function cb, EventPayload *rawPayload) {
      std::unique_ptr<EventPayload> payload(rawPayload);
      if (g_actual_callback.IsEmpty()) {
        return;
      }

      Napi::Value jsType;
      switch (payload->type) {
        case EVENT_CHANGE:
          jsType = Napi::String::New(env, "change");
          break;
        case EVENT_DELETE:
          jsType = Napi::String::New(env, "delete");
          break;
        case EVENT_RENAME:
          jsType = Napi::String::New(env, "rename");
          break;
        case EVENT_CHILD_CREATE:
          jsType = Napi::String::New(env, "child-create");
          break;
        case EVENT_CHILD_CHANGE:
          jsType = Napi::String::New(env, "child-change");
          break;
        case EVENT_CHILD_DELETE:
          jsType = Napi::String::New(env, "child-delete");
          break;
        case EVENT_CHILD_RENAME:
          jsType = Napi::String::New(env, "child-rename");
          break;
        default:
          return;
      }

      g_actual_callback({
        jsType,
        WatcherHandleToV8Value(payload->handle),
        Napi::String::New(env, payload->new_path.data(), payload->new_path.size()),
        Napi::String::New(env, payload->old_path.data(), payload->old_path.size())
      });
    });
}

Napi::Value SetCallback(const Napi::CallbackInfo &info) {
  auto env = info.Env();
  if (!info[0].IsFunction())
    throw Napi::TypeError::New(env, "Function required");

  g_actual_callback.Reset(info[0].As<Napi::Function>(), 1);
  return env.Undefined();
}

Napi::Value Watch(const Napi::CallbackInfo &info) {
  auto env = info.Env();
  if (!info[0].IsString())
    throw Napi::TypeError::New(env, "String required");

  std::string path = info[0].ToString().Utf8Value();
  WatcherHandle handle = PlatformWatch(path.c_str());
  if (!PlatformIsHandleValid(handle)) {
    int error_number = PlatformInvalidHandleToErrorNumber(handle);
    auto err = Napi::Error::New(env, "Unable to watch path");
    if (error_number != 0) {
      err.Set("errno", Napi::Number::New(env, error_number));
      err.Set("code", Napi::String::New(env, uv_err_name(-error_number)));
    }

    throw err;
  }

  return WatcherHandleToV8Value(handle);
}

Napi::Value Unwatch(const Napi::CallbackInfo &info) {
  auto env = info.Env();
  if (!IsV8ValueWatcherHandle(info[0]))
    throw Napi::TypeError::New(env, "Local type required");

  PlatformUnwatch(V8ValueToWatcherHandle(info[0]));

  return env.Undefined();
}
