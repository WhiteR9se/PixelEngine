#include <kernel/game.h>
#include <kernel/pluginManager.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifndef PLUGIN_DIR
  #define PLUGIN_DIR "bin/debug/plugins"
#endif

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
  WindowBackend backend;
} EngineState;

static EngineState state;

static const char* backendPluginFile(WindowBackend BACKEND)
{
  switch (BACKEND)
  {
    case WINDOW_BACKEND_WINDOWED: return "libglfw_window.so";
    case WINDOW_BACKEND_TERMINAL: return "libterminal_window.so";
    case WINDOW_BACKEND_HEADLESS: return "libheadless_window.so";
  }
  return NULL;
}

JUST_API bool gameParseArgs(int argc, char** argv, EngineConfig* config)
{
  bool backendSet = false;

  for (int i = 1; i < argc; ++i)
  {
    WindowBackend requested = config->backend;

    if      (strcmp(argv[i], "--window")   == 0) { requested = WINDOW_BACKEND_WINDOWED; }
    else if (strcmp(argv[i], "--terminal") == 0) { requested = WINDOW_BACKEND_TERMINAL; }
    else if (strcmp(argv[i], "--headless") == 0) { requested = WINDOW_BACKEND_HEADLESS; }
    else { continue; }   // not ours, leave it for the game to parse

    if (backendSet)
    {
      JUST_LOG_ERROR("[ENGINE] : only one of --window, --terminal, --headless is allowed");
      return false;
    }
    config->backend = requested;
    backendSet = true;
  }
  return true;
}

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

  state.backend = GAME->config.backend;
  state.init     = GAME->init;
  state.update   = GAME->update;
  state.shutdown = GAME->shutdown;
  
  JUST_LOG_INFO("[ENGINE] : Game created and initialized");
  return true;
}


JUST_API bool gameRun(void)
{
  JUST_ASSERT_DEBUG_MESSAGE(state.isInitialized == true, "[ENGINE] : Tried to run the game without initializing");

  const char* pluginFile = backendPluginFile(state.backend);
  if (pluginFile == NULL)
  {
    JUST_LOG_ERROR("[ENGINE] : unknown window backend");
    return false;
  }

  char pluginPath[MAX_NAME_LEN];
  int  written = snprintf(pluginPath, sizeof(pluginPath), "%s/%s", PLUGIN_DIR, pluginFile);
  if (written < 0 || (size_t)written >= sizeof(pluginPath))
  {
    JUST_LOG_ERROR("[ENGINE] : window plugin path too long");
    return false;
  }

  if (!pluginManagerLoadWindow(pluginPath))
  {
    JUST_LOG_ERROR("[ENGINE] : Could not load window plugin '%s'", pluginPath);
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
