#define _POSIX_C_SOURCE 199309L   // nanosleep under -std=c11

#include <kernel/windowApi.h>
#include <signal.h>
#include <time.h>

static volatile sig_atomic_t stopRequested = 0;

static void onSignal(int signalNumber)
{
  (void)signalNumber;
  stopRequested = 1;
}

static bool headlessInit(uint32_t width, uint32_t height, const char* title)
{
  (void)width; (void)height; (void)title;
  stopRequested = 0;
  signal(SIGINT,  onSignal);
  signal(SIGTERM, onSignal);
  return true;
}

static void headlessPollEvents(void)
{
  struct timespec pause = { .tv_sec = 0, .tv_nsec = 1000000 };  // 1 ms, avoids 100% CPU
  nanosleep(&pause, NULL);
}

static bool headlessShouldClose(void) { return stopRequested != 0; }

static void headlessShutdown(void)
{
  signal(SIGINT,  SIG_DFL);
  signal(SIGTERM, SIG_DFL);
}

static const WindowAPI api =
{
  .init        = headlessInit,
  .pollEvents  = headlessPollEvents,
  .shouldClose = headlessShouldClose,
  .shutdown    = headlessShutdown
};

JUST_PLUGIN_API const WindowAPI* getWindowAPI(void)
{
  return &api;
} 