#include <kernel/windowApi.h>
#include <GLFW/glfw3.h>
#include <stdio.h>

static GLFWwindow* window = NULL;

static void glfwPluginErrorCallback(int code, const char* description)
{
  fprintf(stderr, "[GLFW PLUGIN] : error %d: %s\n", code, description);
}

static bool glfwPluginInit(uint32_t width, uint32_t height, const char* title)
{
  if (window != NULL) { return false; }

  glfwSetErrorCallback(glfwPluginErrorCallback);

  if (!glfwInit()) { return false; }

  // We only want a window; SDL3 will own rendering later
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  window = glfwCreateWindow((int)width, (int)height, title, NULL, NULL);
  if (window == NULL)
  {
    glfwTerminate();
    return false;
  }
  return true;
}

static void glfwPluginPollEvents(void) { glfwPollEvents(); }

static bool glfwPluginShouldClose(void)
{
  return window == NULL || glfwWindowShouldClose(window);
}

static void glfwPluginShutdown(void)
{
  if (window != NULL)
  {
    glfwDestroyWindow(window);
    window = NULL;
  }
  glfwTerminate();
}

static const WindowAPI api =
{
  .init        = glfwPluginInit,
  .pollEvents  = glfwPluginPollEvents,
  .shouldClose = glfwPluginShouldClose,
  .shutdown    = glfwPluginShutdown
};

JUST_PLUGIN_API const WindowAPI* getWindowAPI(void)
{
  return &api;
}