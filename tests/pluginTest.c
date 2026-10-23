#include <justLibrary.h>
#include <kernel/pluginManager.h>

JustTestResult testPluginSystem(void)
{
  pluginManagerInit();
  justMemorySetLimit(1024 * 1024, "PLUGINS");

  PluginManifest manifest = 
    {
      .author  = "asher",
      .name    = "fib",
      .version = 1
    };
  PluginHandle handle = pluginManagerRegisterPlugin("bin/debug/plugins/libFib.so", manifest);
  JUST_EXPECT_NOT_TO_BE(INVALID_PLUGIN_HANDLE, handle);

  Plugin* fromHandle = pluginManagerGetPlugin(handle);
  Plugin* fromName   = pluginManagerGetPluginByName("fib");

  JUST_EXPECT_TO_BE_NOT_NULL(fromHandle);
  JUST_EXPECT_TO_BE_NOT_NULL(fromName);
  JUST_EXPECT_TO_BE(fromHandle, fromName);
  JUST_EXPECT_TO_BE_TRUE(pluginManagerLoadPlugin(handle));

  typedef int (*FibFunc)(int);
  FibFunc computeFib = (FibFunc)pluginManagerGetSymbol(handle, "fib");

  JUST_EXPECT_TO_BE_NOT_NULL(computeFib);
  JUST_EXPECT_TO_BE(5, computeFib(5));
  JUST_LOG_DEBUG("[FIB PLUGIN] : Fibonnaci of 3 is : %d", computeFib(3));

  pluginManagerShutdown();
  return JUST_TEST_PASS;
}

int main(void)
{
  justTestRegister(testPluginSystem, "Just testing fibbonacci plugin", 0);
  justTestRunAll();
}
