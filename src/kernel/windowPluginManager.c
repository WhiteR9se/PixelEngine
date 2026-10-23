#include <kernel/pluginManager.h>


//---------
//LINUX OS
//---------


#if JUST_PLATFORM == JUST_PLATFORM_LINUX

#include <dlfcn.h>
#include <string.h>

static void* windowHandle = NULL;
static const WindowAPI* windowApi = NULL; 

static bool windowAPIValid(const WindowAPI* API)
{
  return API != NULL
        && API->init        != NULL
        && API->pollEvents  != NULL
        && API->shouldClose != NULL
        && API->shutdown    != NULL;

}

JUST_API bool pluginManagerLoadWindow(const char* PATH)
{
  if (PATH == NULL)
  {
    JUST_LOG_ERROR("[PLUGIN] : window plugin path is NULL");
    return false;
  }

  if (windowHandle != NULL)
  {
    JUST_LOG_ERROR("[PLUGIN] : window plugin already loaded");
    return false;
  }

  void* handle = dlopen(PATH, RTLD_NOW | RTLD_LOCAL);
  if (handle == NULL)
  {
    JUST_LOG_ERROR("[PLUGIN] : dlopen failed for '%s': %s", PATH, dlerror());
    return false;
  }

  dlerror(); // clear any stale error before dlsym
  void*       symbol = dlsym(handle, JUST_WINDOW_API_SYMBOL);
  const char* error  = dlerror();
  if (error != NULL || symbol == NULL)
  {
    JUST_LOG_ERROR("[PLUGIN] : symbol '%s' not found in '%s'", JUST_WINDOW_API_SYMBOL, PATH);
    dlclose(handle);
    return false;
  }

  // memcpy instead of a direct cast: void* -> function pointer is flagged by -Wpedantic
  GetWindowAPIFunc getter;
  memcpy(&getter, &symbol, sizeof(getter));

  const WindowAPI* api = getter();
  if (!windowAPIValid(api))
  {
    JUST_LOG_ERROR("[PLUGIN] : '%s' returned an invalid WindowAPI", PATH);
    dlclose(handle);
    return false;
  }

  windowHandle = handle;
  windowApi    = api;
  return true;
}

JUST_API void pluginManagerUnloadWindow(void)
{
  if (windowHandle != NULL)
  {
    dlclose(windowHandle);
  }
  windowHandle = NULL;
  windowApi    = NULL;
}

JUST_API const WindowAPI* pluginManagerGetWindow(void)
{
  return windowApi;
}



//---------
//WINDOWS OS
//---------


#else

JUST_API bool pluginManagerLoadWindow(const char* PATH)
{
  (void)PATH;
  JUST_LOG_ERROR("[PLUGIN] : window plugins not implemented on this platform");
  return false;
}

JUST_API void pluginManagerUnloadWindow(void) {}

JUST_API const WindowAPI* pluginManagerGetWindow(void) { return NULL; }

#endif
