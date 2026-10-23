#pragma once
#include <justLibrary.h>
#include <stdint.h>

/**
 * @file : windowDriver.h
 * @brief : defines the interface for display plugins
 */

typedef struct windowDriverConfig
{
  const char* title;
  uint32_t    width;
  uint32_t    height;
} WindowDriverConfig;

typedef struct windowDriver
{
  void* driverData;                                                                     ///< Internal state pointer

  bool (*init)        (struct windowDriver* DRIVER, const WindowDriverConfig* CONFIG);  ///< Window initializer
  void (*shutdown)    (struct windowDriver* DRIVER);                                    ///< Window destroy

  void (*pollEvents)  (struct windowDriver* DRIVER);                                    ///< Do keyboard input check
  bool (*shouldClose) (struct windowDriver* DRIVER);                                    ///< Whether the window should close now

  void (*beginFrame)  (struct windowDriver* DRIVER);                                    ///< Ran before frame
  void (*endFrame)    (struct windowDriver* DRIVER);                                    ///< Ran after frame
} WindowDriver;


