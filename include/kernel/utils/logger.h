/**
 * @file logger.h 
 * @brief provides logging utilities to any program, have colored logging output, severity based coloring etc
 * @brief Taken from ForgeLibrary
 * 
 * @warning use stdout and not thread safe, if you want logging in a file, pipe stdout to a file 
 * @see https://github.com/Asher-Ul-Haque/ForgeLibrary
 * @see logger.c 
*/

#pragma once

#include <kernel/defines.h>

#ifdef __cplusplus
extern "C" {
#endif


// - - - | Log macros and defs | - - - 


// - - - Enables - - -

/// @brief The DEBUG macro turns thorough debugging on by default
#ifdef DEBUG 
  #define LOG_WARNING_ENABLED
  #define LOG_DEBUG_ENABLED
  #define LOG_TRACE_ENABLED
  #define LOG_INFO_ENABLED
#endif 

// - - - error and fatal are always enabled
#define LOG_FATAL_ENABLED
#define LOG_ERROR_ENABLED

#ifndef PRINT_LOG_TYPES
  #define PRINT_LOG_TYPES
#endif

#ifndef PRINT_LOG_COLORS
  #define PRINT_LOG_COLORS
#endif


// - - - Log Levels - - -

/// @brief Represents log level, internally
typedef enum LogLevel 
{
  LOG_LEVEL_FATAL   =   0,      /// @brief Just give up and die 
  LOG_LEVEL_ERROR   =   1,      /// @brief Something went wrong badly
  LOG_LEVEL_WARNING =   2,      /// @brief Are you sure about that
  LOG_LEVEL_INFO    =   3,      /// @brief Just some information
  LOG_LEVEL_DEBUG   =   4,      /// @brief Debugging information
  LOG_LEVEL_TRACE   =   5       /// @brief Trace every step, be verbose
} LogLevel;


// - - - API Controls - - -

ENGINE_API void logOutput(LogLevel LEVEL, const char* MESSAGE, ...); // - - - Multivariate, takes any number of arguments greater than 1


// - - - Fatal log
// - - - Always define FATAL and ERROR logs.
#ifdef LOG_FATAL_ENABLED
  #define LOG_FATAL(...) logOutput(LOG_LEVEL_FATAL, __VA_ARGS__, "%s", "");
#endif

#ifdef LOG_ERROR_ENABLED
  #define LOG_ERROR(...) logOutput(LOG_LEVEL_ERROR, __VA_ARGS__, "%s", "");
#endif

// - - - For the rest, define only when enabled, else define to nothingness
#ifdef LOG_WARNING_ENABLED
  #define LOG_WARNING(...) logOutput(LOG_LEVEL_WARNING, __VA_ARGS__, "%s", "");
#else
  #define LOG_WARNING(...)
#endif

#ifdef LOG_INFO_ENABLED
  #define LOG_INFO(...) logOutput(LOG_LEVEL_INFO, __VA_ARGS__, "%s", "");
#else
  #define LOG_INFO(...)
#endif

#ifdef LOG_DEBUG_ENABLED
  #define LOG_DEBUG(...) logOutput(LOG_LEVEL_DEBUG, __VA_ARGS__, "%s", "");
#else
  #define LOG_DEBUG(...)
#endif

#ifdef LOG_TRACE_ENABLED
  #define LOG_TRACE(...) logOutput(LOG_LEVEL_TRACE, __VA_ARGS__, "%s", "");
#else
  #define LOG_TRACE(...)
#endif

#define LOG_CLEAR() printf("\033[H\033[J")


#ifdef __cplusplus
}
#endif
