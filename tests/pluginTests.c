#include <justLibrary.h>
#include <kernel/pluginManager.h>

#ifndef PLUGIN_DIR
  #define PLUGIN_DIR "bin/debug/plugins"
#endif
#define DUMMY_PLUGIN_PATH PLUGIN_DIR "/libdummy_window.so"

JustTestResult testLoadBadPathFails(void)
{
  JUST_EXPECT_TO_BE_FALSE(pluginManagerLoadWindow("does/not/exist.so"));
  JUST_EXPECT_TO_BE_TRUE(pluginManagerGetWindow() == NULL);
  return JUST_TEST_PASS;
}

JustTestResult testLoadDummySucceeds(void)
{
  JUST_EXPECT_TO_BE_TRUE(pluginManagerLoadWindow(DUMMY_PLUGIN_PATH));

  const WindowAPI* win = pluginManagerGetWindow();
  JUST_EXPECT_TO_BE_TRUE(win != NULL);
  JUST_EXPECT_TO_BE_TRUE(win->init        != NULL);
  JUST_EXPECT_TO_BE_TRUE(win->pollEvents  != NULL);
  JUST_EXPECT_TO_BE_TRUE(win->shouldClose != NULL);
  JUST_EXPECT_TO_BE_TRUE(win->shutdown    != NULL);

  pluginManagerUnloadWindow();
  return JUST_TEST_PASS;
}

JustTestResult testDoubleLoadRejected(void)
{
  JUST_EXPECT_TO_BE_TRUE(pluginManagerLoadWindow(DUMMY_PLUGIN_PATH));
  JUST_EXPECT_TO_BE_FALSE(pluginManagerLoadWindow(DUMMY_PLUGIN_PATH));

  pluginManagerUnloadWindow();
  return JUST_TEST_PASS;
}

JustTestResult testDummyLifecycle(void)
{
  JUST_EXPECT_TO_BE_TRUE(pluginManagerLoadWindow(DUMMY_PLUGIN_PATH));
  const WindowAPI* win = pluginManagerGetWindow();

  JUST_EXPECT_TO_BE_TRUE(win->init(800, 600, "test"));
  JUST_EXPECT_TO_BE_FALSE(win->shouldClose());

  for (int i = 0; i < 3; ++i)
  {
    win->pollEvents();
  }
  JUST_EXPECT_TO_BE_TRUE(win->shouldClose());

  win->shutdown();
  pluginManagerUnloadWindow();
  return JUST_TEST_PASS;
}

JustTestResult testUnloadAndReload(void)
{
  JUST_EXPECT_TO_BE_TRUE(pluginManagerLoadWindow(DUMMY_PLUGIN_PATH));
  pluginManagerUnloadWindow();
  JUST_EXPECT_TO_BE_TRUE(pluginManagerGetWindow() == NULL);

  JUST_EXPECT_TO_BE_TRUE(pluginManagerLoadWindow(DUMMY_PLUGIN_PATH));
  JUST_EXPECT_TO_BE_TRUE(pluginManagerGetWindow() != NULL);

  pluginManagerUnloadWindow();
  return JUST_TEST_PASS;
}

int main(void)
{
  justTestRegister(testLoadBadPathFails, "Plugin: bad path fails", 1);   // true on every platform

#if JUST_PLATFORM == JUST_PLATFORM_LINUX
  justTestRegister(testLoadDummySucceeds,  "Plugin: dummy loads",            1);
  justTestRegister(testDoubleLoadRejected, "Plugin: double load rejected",   1);
  justTestRegister(testDummyLifecycle,     "Plugin: dummy 3-tick lifecycle", 1);
  justTestRegister(testUnloadAndReload,    "Plugin: unload then reload",     1);
#endif

  return justTestRunAll();
}
