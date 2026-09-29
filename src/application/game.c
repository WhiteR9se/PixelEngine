#include <kernel/game.h>
#include <stdint.h>
#include <string.h>

#define MAX_NAME_LEN 1024

typedef struct engineState
{
  bool isInitialized : 1;
  bool isRunning     : 1;
  bool isSuspended   : 1;

  uint32_t  height;
  uint32_t  width;

  double    lastTime;

  char name[MAX_NAME_LEN];
} EngineState;

static EngineState state;


JUST_API bool gameCreate(const Game* GAME)
{
  JUST_ASSERT_DEBUG_MESSAGE(state.isInitialized == false, "[ENGINE] : Tried to create a GAME twice");
  JUST_ASSERT_DEBUG_MESSAGE(GAME->config.name != NULL, "[ENGINE] : GAME name not provided");
  JUST_ASSERT_DEBUG_MESSAGE(strlen(GAME->config.name) < MAX_NAME_LEN, "[ENGINE] : GAME name too big");
  JUST_ASSERT_DEBUG_MESSAGE(GAME->config.height > 0, "[ENGINE] : Cannot make a GAME with 0 height");
  JUST_ASSERT_DEBUG_MESSAGE(GAME->config.width > 0, "[ENGINE] : Cannot make a GAME with 0 width");

  strncpy(state.name, GAME->config.name, MAX_NAME_LEN - 1);
  state.height        = GAME->config.height;
  state.width         = GAME->config.width;
  state.isRunning     = false;
  state.isSuspended   = false;
  state.isInitialized = true;

  JUST_LOG_INFO("[ENGINE] : Game created and initialized");
  return true;
}

JUST_API bool gameRun(void)
{
  JUST_ASSERT_DEBUG_MESSAGE(state.isInitialized == true, "[ENGINE] : Tried to run the game without initializing");
  TODO
}
