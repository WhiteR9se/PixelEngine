#include <kernel/game.h>
#include <kernel/pluginManager.h>
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

  GameInitFunc init;
  GameUpdateFunc update;
  GameShutdownFunc shutdown;

  char name[MAX_NAME_LEN];
  char windowPlugin[MAX_NAME_LEN];
} EngineState;

static EngineState state;


JUST_API bool gameCreate(const Game* GAME)
{
  JUST_ASSERT_DEBUG_MESSAGE(state.isInitialized == false, "[ENGINE] : Tried to create a GAME twice");
  JUST_ASSERT_DEBUG_MESSAGE(GAME->config.name != NULL, "[ENGINE] : GAME name not provided");
  JUST_ASSERT_DEBUG_MESSAGE(strlen(GAME->config.name) < MAX_NAME_LEN, "[ENGINE] : GAME name too big");
  JUST_ASSERT_DEBUG_MESSAGE(GAME->config.height > 0, "[ENGINE] : Cannot make a GAME with 0 height");
  JUST_ASSERT_DEBUG_MESSAGE(GAME->config.width > 0, "[ENGINE] : Cannot make a GAME with 0 width");
  JUST_ASSERT_DEBUG_MESSAGE(GAME->config.windowPlugin != NULL, "[ENGINE] : GAME window plugin path not provided");

  strncpy(state.name, GAME->config.name, MAX_NAME_LEN - 1);
  state.height        = GAME->config.height;
  state.width         = GAME->config.width;
  state.isRunning     = false;
  state.isSuspended   = false;
  state.isInitialized = true;

  strncpy(state.windowPlugin, GAME->config.windowPlugin, MAX_NAME_LEN - 1);
  state.init     = GAME->init;
  state.update   = GAME->update;
  state.shutdown = GAME->shutdown;
  
  JUST_LOG_INFO("[ENGINE] : Game created and initialized");
  return true;
}
JUST_API bool gameRun(void)
{
  JUST_ASSERT_DEBUG_MESSAGE(state.isInitialized == true, "[ENGINE] : Tried to run the game without initializing");

  if (!pluginManagerLoadWindow(state.windowPlugin))
  {
    JUST_LOG_ERROR("[ENGINE] : Could not load window plugin '%s'", state.windowPlugin);
    return false;
  }

  const WindowAPI* win = pluginManagerGetWindow();

  if (!win->init(state.width, state.height, state.name))
  {
    JUST_LOG_ERROR("[ENGINE] : Window plugin failed to initialize");
    pluginManagerUnloadWindow();
    return false;
  }

  if (state.init != NULL) { state.init(); }
  state.isRunning = true;

  while (state.isRunning && !win->shouldClose())
  {
    win->pollEvents();
    if (state.update != NULL) { state.update(); }
  }

  if (state.shutdown != NULL) { state.shutdown(); }
  win->shutdown();
  pluginManagerUnloadWindow();
  state.isRunning = false;
  return true;
}
