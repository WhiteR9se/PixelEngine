#pragma once

#include <kernel/justLibrary.h>
#include <stdint.h>


typedef void (*GameInitFunc) (void);
typedef void (*GameShutdownFunc) (void);
typedef void (*GameUpdateFunc) (void);

typedef enum windowBackend
{
  WINDOW_BACKEND_WINDOWED = 0,   // default plugin. windowed game engine. :)
  WINDOW_BACKEND_TERMINAL,
  WINDOW_BACKEND_HEADLESS
} WindowBackend;

typedef struct engineConfig
{
  uint32_t      height;
  uint32_t      width;
  uint32_t      xPos;
  uint32_t      yPos;
  const char*   name;
  WindowBackend backend;        // which window plugin to load (--window / --terminal / --headless)
} EngineConfig;

typedef struct game
{
  GameInitFunc      init;
  GameShutdownFunc  shutdown;
  GameUpdateFunc    update;
  EngineConfig      config;
} Game;


JUST_API bool gameCreate(const Game* GAME);
JUST_API bool gameRun(void);
JUST_API bool gameParseArgs(int argc, char** argv, EngineConfig* config);