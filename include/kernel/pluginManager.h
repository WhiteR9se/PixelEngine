#pragma once

#include <justLibrary.h>
#include <kernel/windowApi.h>
#include <stdint.h>

JUST_API bool pluginManagerLoadWindow(const char* PATH);
JUST_API void pluginManagerUnloadWindow(void);
JUST_API const WindowAPI* pluginManagerGetWindow(void);

typedef struct enginePluginManifest
{
  const char* name;
  const char* author;
  uint32_t    version;
} PluginManifest;

typedef uint32_t PluginHandle;
#define INVALID_PLUGIN_HANDLE UINT32_MAX

typedef struct enginePlugin
{
  char            path[256];      ///< Where is this shared library
  PluginManifest  manifest;       ///< Plugin info
  void*           soHandle;       ///< Shared object handle 
} Plugin;

JUST_API void pluginManagerInit(void);

JUST_API void pluginManagerShutdown(void);

JUST_API PluginHandle pluginManagerRegisterPlugin(const char* PATH, PluginManifest MANIFEST);

JUST_API bool pluginManagerLoadPlugin(PluginHandle HANDLE);

JUST_API Plugin* pluginManagerGetPlugins(void);

JUST_API Plugin* pluginManagerGetPlugin(PluginHandle HANDLE);

JUST_API Plugin* pluginManagerGetPluginByName(const char* NAME);

JUST_API void* pluginManagerGetSymbol(PluginHandle HANDLE, const char* SYMBOL_NAME);
