#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// - - - Platform detection - - -

#define ENGINE_PLATFORM_UNKOWN  0
#define ENGINE_PLATFORM_WINDOWS 1
#define ENGINE_PLATFORM_LINUX   2
#define ENGINE_PLATFORM_ANDROID 3
#define ENGINE_PLATFORM_APPLE   4

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
  #define ENGINE_PLATFORM ENGINE_PLATFORM_WINDOWS
#elif defined(__linux__) || defined(__gnu_linux__)
  #define ENGINE_PLATFORM ENGINE_PLATFORM_LINUX
#elif defined(__ANDROID__)
  #define ENGINE_PLATFORM ENGINE_PLATFORM_ANDROID
#elif defined(__APPLE__)
  #define ENGINE_PLATFORM ENGINE_PLATFORM_APPLE
#else
  #define ENGINE_PLATFORM ENGINE_PLATFORM_UNKOWN
#endif

#if ENGINE_PLATFORM == ENGINE_PLATFORM_WINDOWS
  #if defined(ENGINE_EXPORT)
    #define ENGINE_API __attribute__((dllexport))
  #else
    #define ENGINE_API __attribute__((dllimport))
  #endif
  #define ENGINE_LOCAL
#else
  #define ENGINE_API __attribute__((visibility("default")))
  #define ENGINE_LOCAL __attribute__((visibility("hidden")))
#endif

#ifdef __cplusplus
}
#endif
