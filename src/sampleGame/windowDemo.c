#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <kernel/game.h>

static void demoUpdate(void)
{
  static uint64_t ticks         = 0;
  static uint64_t ticksAtPrint  = 0;
  static double   lastPrintTime = 0.0;

  ticks++;

  struct timespec now;
  timespec_get(&now, TIME_UTC);   // standard C11, no feature macros needed
  double seconds = (double)now.tv_sec + (double)now.tv_nsec / 1e9;

  if (lastPrintTime == 0.0) { lastPrintTime = seconds; }

  if (seconds - lastPrintTime >= 1.0)
  {
    printf("%llu ticks in the last second\n", (unsigned long long)(ticks - ticksAtPrint));
    ticksAtPrint  = ticks;
    lastPrintTime = seconds;
  }
}

int main(int argc, char** argv)
{
  Game game =
  {
    .init     = NULL,
    .update   = demoUpdate,
    .shutdown = NULL,
    .config   =
    {
      .height  = 600,
      .width   = 800,
      .xPos    = 0,
      .yPos    = 0,
      .name    = "PixelEngine",
      .backend = WINDOW_BACKEND_WINDOWED
    }
  };

  if (!gameParseArgs(argc, argv, &game.config)) { return 1; }
  if (!gameCreate(&game))                       { return 1; }
  if (!gameRun())                               { return 1; }
  return 0;
}