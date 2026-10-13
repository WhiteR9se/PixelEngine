#include <kernel/windowApi.h>

static uint32_t pollCount = 0;

static bool dummyInit(uint32_t width, uint32_t height, const char* title)
{
  (void)width; (void)height; (void)title;
  pollCount = 0;
  return true;
}

static void dummyPollEvents(void) { pollCount++; }
static bool dummyShouldClose(void) { return pollCount >= 3; }
static void dummyShutdown(void) { pollCount = 0; }

static const WindowAPI api =
{
  .init        = dummyInit,
  .pollEvents  = dummyPollEvents,
  .shouldClose = dummyShouldClose,
  .shutdown    = dummyShutdown
};

JUST_PLUGIN_API const WindowAPI* getWindowAPI(void)
{
  return &api;
}