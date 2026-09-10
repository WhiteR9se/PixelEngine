/**
 * @file : tracker.h 
 * @brief : Overwrites of malloc, realloc and free to have memory tracking in debug mode
 * @brief : Taken from forgeLibrary
 * @see: https://github.com/Asher-Ul-Haque/ForgeLibrary 
*/

#pragma once

#include <kernel/defines.h>
#include <stddef.h>
#include <stdint.h>
#include <memory.h>
#include <stdlib.h>

#ifdef __cplusplus
extern C {
#endif

/// @brief : Represents the default limit in bytes for any tag for allocation (1KiB)
#define MEMORY_LIMIT_DEFAULT 1024

/// @brief : Enum that stores various reasons for allocating memory
typedef enum memoryTag
{
  MEMORY_TAG_UNKOWN,        ///< I don't know why I am allocating
  MEMORY_TAG_ARRAY,         ///< I want to make an array
  MEMORY_TAG_DYNAMIC_ARRAY, ///< I want to make a dynamic array
  MEMORY_TAG_STRING,        ///< I want to make a string
  MEMORY_TAG_ECS_COMPONENT, ///< I want to make a component
  MEMORY_TAG_COUNT          ///< Represents total
} MemoryTag;

/**
 * @brief : Allocates memory with surrounding magic canary guards and tracking metadata.
 * @param SIZE : how much to allocate 
 * @param FILE : which file is it allocated in 
 * @param FUNCTION : Which function is it allocated in
 * @param LINE : Which line is it allocated in
 * @param TAG : Why are you allocating the memory
 * @warning : Do not use directly, this is not supposed to be used, use the macro version instead
 * @see : ENGINE_MALLOC
 */
ENGINE_API void* memoryTrackedMalloc(
  size_t      SIZE,
  const char* FILE,
  const char* FUNC,
  int32_t     LINE,
  MemoryTag   TAG);

/**
 * @brief : Reallocates memory, updating canary guards and size tracking.
 * @param PTR : Ptr to realloc 
 * @param NEW_SIZE : new allocation size
 * @param FILE : What file is the reallocate in 
 * @param FUNCTION : What function is the reallocate in 
 * @param LINE : What line is the reallocate in
 * @warning : Unlike realloc, passing NULL to it will fail, instead of malloc behavior
 * @warning : Do not use directly, this is not supposed to be used, use the macro version instead
 * @see : ENGINE_REALLOC
*/
ENGINE_API void* memoryTrackedRealloc(
  void*       PTR,
  size_t      NEW_SIZE,
  const char* FILE,
  const char* FUNCTION,
  int32_t     LINE);

/**
 * @brief : Allocates zero-initialized memory with canary guards and tracking metadata.
 * @param COUNT : How many objects 
 * @param SIZE : Size of one object 
 * @param FILE : What file is the callocate in 
 * @param FUNCTION : What function is the callocate in 
 * @param LINE : What line is the callocate in
 * @param TAG : Why are you allocating the memory
 * @warning : zeroes out the memory
 * @warning : Do not use directly, this is not supposed to be used, use the macro version instead
 * @see : ENGINE_CALLOC
 */
ENGINE_API void* memoryTrackedCalloc(
  size_t      COUNT,
  size_t      SIZE,
  const char* FILE,
  const char* FUNC,
  int32_t     LINE,
  MemoryTag   TAG);

/**
 * @brief : Frees memory and verifies Canary safety bounds.
 * @param PTR : What ptr to free 
 * @param FILE : What file is free called in 
 * @param FUNCTION : What function is the free in 
 * @param LINE : What line is the free in
 * @warning : Do not use directly, this is not supposed to be used, use the macro version instead
 * @see : ENGINE_FREE
 */
ENGINE_API void memoryTrackedFree(
  void*       PTR,
  const char* FILE,
  const char* FUNCTION,
  int32_t     LINE);

/**
 * @brief : Validates all active allocations against canary corruption.
 * @return : true if all allocations are intact, false if corruption detected.
 */
ENGINE_API bool memoryCheckBounds(void);

/** @brief : Reports all active allocations that haven't been freed (Memory Leaks).
 * @warning : just for debugging, not for actually parsing leaks 
 */
ENGINE_API void memoryReportLeaks(void);

/**
 * @brief : Returns total active allocated bytes currently in use.
 * @param TAG : The tag for which you want to check memory, set it to MEMORY_TAG_COUNT to get all
 * @return : total active allocated bytes in use
 */
ENGINE_API size_t memoryGetActiveBytes(MemoryTag TAG);

/**
 * @brief : Returns the memory allocated as a string,
 * @param VERBOSE : If true, then every information is printed, where did each allocation happen, line, file and function
 * @warning : just for debugging, not for actually parsing memory usage
 * @see : memoryGetActiveBytes for better usage API
*/
ENGINE_API void memoryLogUsageStr(bool VERBOSE);

/**
 * @brief : Sets a limit on the memory allocation of a particular type
 * @warning : If you exceed the limit, the allocations will return NULL
 * @warning : TAG must be valid
 * @param LIMIT : The limit you want to set in bytes
 * @param TAG : What do you want to set the limit for
*/
ENGINE_API void memorySetLimit(size_t LIMIT, MemoryTag TAG);

/**
 * @brief : Returns the memory allocation limit
 * @warning : TAG must be valid
*/
ENGINE_API size_t memoryGetLimit(MemoryTag TAG);


// - - - | MACRO Usage API | - - -

/// @brief : Simple Macros to use, recommended to use these over the functions directly
#ifdef DEBUG
  #define ENGINE_MALLOC(size, tag)          memoryTrackedMalloc((size), __FILE__, __func__, __LINE__, tag)
  #define ENGINE_REALLOC(ptr, size)         memoryTrackedRealloc((ptr), (size), __FILE__, __func__, __LINE__)
  #define ENGINE_CALLOC(count, size, tag)   memoryTrackedCalloc((count), (size), __FILE__, __func__, __LINE__, tag)
  #define ENGINE_FREE(ptr)                  memoryTrackedFree((ptr), __FILE__, __func__, __LINE__)
#else 
  #define ENGINE_MALLOC(size, tag)        malloc(size)
  #define ENGINE_REALLOC(ptr, size)       realloc(ptr, size)
  #define ENGINE_CALLOC(count, size, tag) calloc((count), (size))
  #define ENGINE_FREE(ptr)                free(ptr)
#endif

