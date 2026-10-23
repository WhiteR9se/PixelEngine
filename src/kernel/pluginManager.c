#include <justLibrary.h>
#include <dlfcn.h>
#include <kernel/pluginManager.h>

static const char*      TEMP_DIR    = "./.plugin_temp";
static bool             init        = false;
static JustDynamicArray plugins;

JUST_API void pluginManagerInit(void)
{
  JUST_LOG_DEBUG("[PLUGIN MANAGER] : Initializing...");
  JUST_ASSERT_DEBUG_MESSAGE(!init, "[PLUGIN MANAGER] : Already initialized");

  // - - - Initialize the dynamic array
  bool ok = JUST_DARRAY_INIT_TAGGED(&plugins, 0, Plugin, "PLUGINS");
  JUST_ASSERT_DEBUG(ok);

  // - - - Ensure the temporary directory exists
  if (!justIsDirectory(TEMP_DIR))
  {
    ok = justMkdirRecursive(TEMP_DIR);
    JUST_ASSERT_DEBUG(ok);
  }

  init = true;
  JUST_LOG_INFO("[PLUGIN MANAGER] : Plugin manager initialized");
}

JUST_API void pluginManagerShutdown(void)
{
  JUST_LOG_DEBUG("[PLUGIN MANAGER] : Shutting down...");
  JUST_ASSERT_DEBUG_MESSAGE(init, "[PLUGIN MANAGER] : Cannot shutdown if never initialized");

  size_t  count       = justDynamicArraySize(&plugins);
  Plugin* pluginData  = JUST_DARRAY_DATA(&plugins, Plugin);

  // - - - close all active dynamic librarys
  for (size_t i = 0; i < count; ++i)
  {
    if (pluginData[i].soHandle)
    {
      dlclose(pluginData[i].soHandle);
      pluginData[i].soHandle = NULL;
    }
  }

  // - - - delete the dynamic array and temp files
  justDynamicArrayDestroy(&plugins);
  justRmdirRecursive(TEMP_DIR);

  init = false;
  JUST_LOG_INFO("[PLUGIN MANAGER] : Plugin Manager shut down");
}

JUST_API PluginHandle pluginManagerRegisterPlugin(const char* PATH, PluginManifest MANIFEST)
{
  JUST_ASSERT_DEBUG_MESSAGE(init, "[PLUGIN MANAGER] : Cannot call register before init is called");
  JUST_ASSERT_DEBUG_MESSAGE(PATH != NULL, "[PLUGIN MANAGER] : Cannot open NULL PATH");

  Plugin* plug = JUST_DARRAY_EMPLACE(&plugins, Plugin);
  plug->manifest = MANIFEST;
  plug->soHandle = NULL;
  snprintf(plug->path, sizeof(plug->path), "%s", PATH);

  PluginHandle handle = (PluginHandle)(justDynamicArraySize(&plugins) - 1);
  JUST_LOG_DEBUG("[PLUGIN MANAGER] : Registered plugin handle %u for path: %s", handle, PATH);

  return handle;
}

JUST_API bool pluginManagerLoadPlugin(PluginHandle HANDLE)
{
  JUST_ASSERT_DEBUG_MESSAGE(init, "[PLUGIN MANAGER] : Manager not initialized yet");
  JUST_ASSERT_DEBUG_MESSAGE(HANDLE < justDynamicArraySize(&plugins), "[PLUGIN MANAGER] : Invalid HANDLE, pass only PluginHandle");


  // - - - If currently loaded, close and old handle
  Plugin* plug = (Plugin*) justDynamicArrayAt(&plugins, HANDLE);
  if (plug->soHandle != NULL)
  {
    dlclose(plug->soHandle);
    plug->soHandle = NULL;
  }

  // - - - Build sandbox temp copy path
  char tempPath[512];
  snprintf(tempPath, sizeof(tempPath), "%s/temp_plugin_%u.so", TEMP_DIR, HANDLE);

  // - - - Copy source .so file into temp directory
  size_t fileSize = 0;
  void*  fileData = justFileReadEntire(plug->path, &fileSize, "PLUGINS");
  if (!fileData)
  {
    JUST_LOG_ERROR("[PLUGIN MANAGER] : Failed to read source plugin file : %s", plug->path);
    return false;
  }

  bool written = justFileWriteEntire(tempPath, fileData, fileSize);
  JUST_FREE(fileData);

  if (!written)
  {
    JUST_LOG_ERROR("[PLUGIN MANAGER] : Failed to copy plugin to temp path: %s", tempPath);
    return false;
  }

  // - - - Open shared library via dlopen
  dlerror();
  void* soHandle = dlopen(tempPath, RTLD_NOW | RTLD_LOCAL);
  if (!soHandle)
  {
    JUST_LOG_ERROR("[PLUGIN MANAGER] : dlopen failed for '%s': %s", tempPath, dlerror());
    return false;
  }
  plug->soHandle = soHandle;

  return true;
}

JUST_API Plugin* pluginManagerGetPlugins(void)
{
  JUST_ASSERT_DEBUG(init);
  if (justDynamicArrayIsEmpty(&plugins)) return NULL;

  return JUST_DARRAY_DATA(&plugins, Plugin);
}

JUST_API Plugin* pluginManagerGetPlugin(PluginHandle HANDLE)
{
  JUST_ASSERT_DEBUG(init);
  if (justDynamicArrayIsEmpty(&plugins)) return NULL;

  return (Plugin*) justDynamicArrayAt(&plugins, HANDLE);
}

JUST_API Plugin* pluginManagerGetPluginByName(const char* NAME)
{
  JUST_ASSERT_DEBUG(init);

  size_t  count = justDynamicArraySize(&plugins);
  Plugin* data  = JUST_DARRAY_DATA(&plugins, Plugin);

  for (size_t i = 0; i < count; ++i)
  {
    if (data[i].manifest.name && strcmp(data[i].manifest.name, NAME) == 0)
    {
      return &data[i];
    }
  }

  return NULL;
}

JUST_API void* pluginManagerGetSymbol(PluginHandle HANDLE, const char* SYMBOL_NAME)
{
  JUST_ASSERT_DEBUG_MESSAGE(init, "[PLUGIN MANAGER] : Manager not initialized");
  JUST_ASSERT_DEBUG_MESSAGE(HANDLE < justDynamicArraySize(&plugins), "[PLUGIN MANAGER] : Invalid HANDLE");
  JUST_ASSERT_DEBUG_MESSAGE(SYMBOL_NAME != NULL, "[PLUGIN MANAGER] : Symbol name cannot be NULL");

  Plugin* plug = (Plugin*)justDynamicArrayAt(&plugins, HANDLE);
  if (!plug->soHandle)
  {
    JUST_LOG_ERROR("[PLUGIN MANAGER] : Attempted to get symbol '%s' from unloaded plugin (handle %u)", SYMBOL_NAME, HANDLE);
    return NULL;
  }

  dlerror();
  void*       symbol  = dlsym(plug->soHandle, SYMBOL_NAME);
  const char* err     = dlerror();
  if (err)
  {
    JUST_LOG_ERROR("[PLUGIN MANAGER] : Failed to find symbol '%s': %s", SYMBOL_NAME, err);
    return NULL;
  }

  return symbol;
}
