# Milestone: Dynamic Window Plugin System

## Step 0: Build System Prep (no plugin files exist yet)
- [ ] Add `PLUGIN_SRCS := $(shell find $(SRC_DIR)/plugins -name '*.c')` (create empty `src/plugins/` first to avoid a `find` error).
- [ ] Add `$(PLUGIN_SRCS)` to the `filter-out` in `COMMON_SRCS` and to `ALL_SRCS` (Linux AND Windows branches).
- [ ] Add `-ldl` to the end of the `$(KERNEL_LIB)` link line (Linux).
- [ ] Verify `make remake` and `make runTests` still pass (nothing new is built yet).

## Step 1: Plugin Contract (`windowApi.h`)
- [ ] Create `include/kernel/windowApi.h` (no dependency on kernel macros).
- [ ] Define `JUST_PLUGIN_API` (dllexport on Windows, visibility("default") otherwise).
- [ ] Define `WindowAPI` struct: `init`, `pollEvents`, `shouldClose`, `shutdown`.
- [ ] Define `GetWindowAPIFunc` and `JUST_WINDOW_API_SYMBOL "getWindowAPI"`.

## Step 2: The Loader (`pluginManager.h` / `pluginManager.c`)
- [ ] Create `include/kernel/pluginManager.h` (load / unload / get).
- [ ] Create `src/kernel/pluginManager.c` with `#if JUST_PLATFORM == JUST_PLATFORM_LINUX` guard.
- [ ] Load: reject if already loaded; `dlopen(path, RTLD_NOW | RTLD_LOCAL)`; log `dlerror()` on failure.
- [ ] Call `dlerror()` to clear stale errors, then `dlsym`; copy into the function pointer with `memcpy` (`-Wpedantic` safe).
- [ ] Validate returned API and all 4 function pointers are non-NULL; on any failure `dlclose` and reset state.
- [ ] Unload: `dlclose`, reset handle and API to NULL. Get: return the API pointer.
- [ ] `#else` block: stubs that log "not implemented" and return false/NULL.
- [ ] Verify `make remake` still passes.

## Step 3: Dummy Plugin, Build Rules & Loader Tests
- [ ] Create `src/plugins/dummy/dummyWindow.c` (4 functions, tick counter, `shouldClose` true after 3 polls).
- [ ] Export `getWindowAPI` with `JUST_PLUGIN_API`, returning a `static const WindowAPI`.
- [ ] Makefile: add `DUMMY_OBJS`, `PLUGIN_OUT`, `DUMMY_PLUGIN` and its `-shared` link rule.
- [ ] Makefile: make `tests` depend on `$(DUMMY_PLUGIN)`.
- [ ] Create `tests/pluginTests.c` following the `coreTests.c` pattern.
- [ ] Decide how tests find the `.so` (relative to repo root, or a `-DPLUGIN_DIR` define); don't hardcode `debug`.
- [ ] Tests: bad path fails; dummy loads with all pointers set; double-load rejected; init + 3 polls + `shouldClose`; unload gives NULL; reload after unload works.
- [ ] Verify `make runTests` passes (no GLFW needed).

## Step 4: Game Loop Refactor
- [ ] `git mv src/application/game.c src/kernel/game.c` (picked up by `KERNEL_SRCS` automatically).
- [ ] Add `const char* windowPlugin;` to `EngineConfig` in `game.h`.
- [ ] `gameCreate`: assert path non-NULL; `strncpy` the path into `EngineState`; store `init`, `update`, `shutdown` callbacks.
- [ ] `gameRun`: remove `TODO`; load plugin; get API; `win->init(...)` (unload + return false on failure).
- [ ] `gameRun`: null-check callbacks; call game init; loop `while (isRunning && !win->shouldClose())` { `pollEvents`; update }.
- [ ] `gameRun`: call game shutdown, `win->shutdown()`, unload plugin, set `isRunning = false`, return true.
- [ ] Verify `make remake` and `make runTests` still pass.

## Step 5: GLFW Plugin
- [ ] Install `libglfw3-dev`.
- [ ] Create `src/plugins/glfw/glfwWindow.c` with a `static GLFWwindow* window`.
- [ ] `init`: `glfwSetErrorCallback` first; `glfwInit`; hint `GLFW_CLIENT_API = GLFW_NO_API`; `glfwCreateWindow`; on NULL call `glfwTerminate` and return false.
- [ ] `pollEvents` = `glfwPollEvents` (no SwapBuffers); `shouldClose` = `glfwWindowShouldClose`.
- [ ] `shutdown`: `glfwDestroyWindow`, `glfwTerminate`, `window = NULL`.
- [ ] Export `getWindowAPI` with `JUST_PLUGIN_API` (`static const WindowAPI`).
- [ ] Makefile: add `GLFW_OBJS`, `GLFW_PLUGIN` with `-shared ... -lglfw` link rule.
- [ ] Makefile: add `plugins` target (GLFW + dummy) and make `all` depend on it.
- [ ] Verify `make remake` and `make runTests` pass.

## Step 6: The Demo
- [ ] Create `src/sampleGame/windowDemo.c`.
- [ ] Add it to `MAIN_SRCS` (Linux; Windows branch optional).
- [ ] Build a full `Game` struct: 800x600, name, `windowPlugin = "bin/debug/plugins/libglfw_window.so"`; callbacks may be NULL.
- [ ] Call `gameCreate(&game)` then `gameRun()`.
- [ ] Run from repo root on a machine with a display; verify the window opens, stays up, and closes cleanly.