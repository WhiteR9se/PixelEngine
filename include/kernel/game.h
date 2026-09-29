#pragma once

#include <kernel/justLibrary.h>
#include <stdint.h>


typedef void (*GameInitFunc) (void);
typedef void (*GameShutdownFunc) (void);
typedef void (*GameUpdateFunc) (void);

typedef struct engineConfig
{
  uint32_t    height;
  uint32_t    width;
  uint32_t    xPos;
  uint32_t    yPos;
  const char* name;
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
