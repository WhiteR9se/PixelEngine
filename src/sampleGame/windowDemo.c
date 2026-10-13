#include <kernel/game.h>

int main(void)
{
  Game game =
  {
    .init     = NULL,
    .update   = NULL,
    .shutdown = NULL,
    .config   =
    {
      .height       = 600,
      .width        = 800,
      .xPos         = 0,
      .yPos         = 0,
      .name         = "PixelEngine",
      .windowPlugin = "bin/debug/plugins/libglfw_window.so" //hardcodes to bin/debug so will only work in debug mode in make. change later.
    }
  };

  if (!gameCreate(&game)) { return 1; }
  if (!gameRun())         { return 1; }
  return 0;
}