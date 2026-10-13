#pragma once

#include <kernel/justLibrary.h>
#include <kernel/windowApi.h>

JUST_API bool pluginManagerLoadWindow(const char* PATH);
JUST_API void pluginManagerUnloadWindow(void);
JUST_API const WindowAPI* pluginManagerGetWindow(void);