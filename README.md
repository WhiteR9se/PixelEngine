# PixelEngine
Retro 2D Game Engine Build Entirely in C 

---

# SRS

## Features
### MVP Features
* A 2D Renderer
* Keyboard and Mouse Control support
* File IO
* Linux Desktop
* Ships with a Pong tutorial
* Have a release

### Necessary Features:
* ECS
* Platform system (platform specific code)
* Event systems
* Game Engine Editor
* Hot Reloading of Assets (including Game Code)
* Plugin system for .so files
* Platforms
* Game Controller Support
* Aseprite Support
* Anything that is possible to do in Scratch, must be possible here
* Serialization : both human readable and binary format

## Constraints
### Platform
* Platforms to support for sure: Linux Desktop, Windows Desktop
* Platforms probably will support : Macos Desktop, Web Assembly
* Platforms want to support: Consoles : Android
* Platforms will not support : Consoles, XBOX, Playstation, iOS, Samsung Fridge

### Coding Environment
* C programming language only (bash / python . make for scripts)
* C standard : C11
* Compiler : clang
* Warnings : -Wall, -Wpedantic, -Werror
* Test driven development (on atleast Windows and Linux)
* Makefile, no CMake
* No AI agentic code, No AI copy paste code
* Fine to use AI to ask questions, even generate code, but read with eyes and type with fingers
* Human Review always

### System Limitation
* Integrated GPUs are fine
* Terminal CPU rendering included

### Libraries
* ImGUI to make the editor
* GLFW to make windows and input output
* SDL3 for rendering and audio
* Physics has Box2D
* Doxygen for documentation 
* Perf for analysis
* GDB for debugging

## Milestones
1. Get a kernel of the game engine up 
  a) ECS
  b) Plugin
  c) IO
  d) Build system for linux
2. Get the first triangle on the screen
  a) SDL3 plugin
  b) Simple 2D Rendering
