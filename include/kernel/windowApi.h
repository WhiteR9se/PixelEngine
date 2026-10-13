#pragma once

#include <stdbool.h>
#include <stdint.h>

#if defined(_WIN32)
  #define JUST_PLUGIN_API __declspec(dllexport)
#else
  #define JUST_PLUGIN_API __attribute__((visibility("default")))
#endif

typedef struct windowAPI
{
  bool (*init)(uint32_t width, uint32_t height, const char* title);
  void (*pollEvents)(void);
  bool (*shouldClose)(void);
  void (*shutdown)(void);
} WindowAPI;

#define JUST_WINDOW_API_SYMBOL "getWindowAPI"

typedef const WindowAPI* (*GetWindowAPIFunc)(void);