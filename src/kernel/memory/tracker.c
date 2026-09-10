#include <kernel/defines.h>
#include <kernel/memory/tracker.h>
#include <kernel/utils/assert.h>
#include <kernel/utils/logger.h>
#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// - - - Internal Structs - - -

/**
 * @brief : Struct that keeps track of allocations as a doubly linked list
 * @warning : Aligned to 16 bytes
*/
typedef struct memoryHeader
{
  alignas(max_align_t) size_t requestedSize; ///< How much did the user ask to allocate
  const char*                 file;          ///< What file was the allocation in
  const char*                 func;          ///< What function was the allocation in
  struct memoryHeader*        next;          ///< Pointer to the next allocation
  struct memoryHeader*        prev;          ///< Pointer to the previous allocation
  uint32_t                    magic;         ///< Magic number, if overwritten, then there is a corruption
  int32_t                     line;          ///< What line was the allocation in
  MemoryTag                   tag;           ///< What was the allocation for
} MemoryHeader;

COMPILE_TIME_ASSERT(alignof(MemoryHeader) == alignof(max_align_t));

static size_t         memoryTagAllocatedBytes[MEMORY_TAG_COUNT]   = {0};
static MemoryHeader*  memoryActiveAllocations                     = NULL;
static const char*    memoryTagStrings[MEMORY_TAG_COUNT]          =
  {
    [MEMORY_TAG_ARRAY]          = "ARRAY        ",
    [MEMORY_TAG_DYNAMIC_ARRAY]  = "DYNAMIC ARRAY",
    [MEMORY_TAG_STRING]         = "STRING       ",
    [MEMORY_TAG_ECS_COMPONENT]  = "ECS COMPONENT",
    [MEMORY_TAG_UNKOWN]         = "UNKOWN       ",
  };
static size_t         memoryTagAllocationLimit[MEMORY_TAG_COUNT]  =
  {
    [MEMORY_TAG_ARRAY]         = MEMORY_LIMIT_DEFAULT,
    [MEMORY_TAG_DYNAMIC_ARRAY] = MEMORY_LIMIT_DEFAULT,
    [MEMORY_TAG_STRING]        = MEMORY_LIMIT_DEFAULT,
    [MEMORY_TAG_ECS_COMPONENT] = MEMORY_LIMIT_DEFAULT,
    [MEMORY_TAG_UNKOWN]        = MEMORY_LIMIT_DEFAULT,
  };

// - - - Magic numbers
#define MEMORY_HEADER_MAGIC 0xCAFEBABEU
#define MEMORY_FOOTER_MAGIC 0xDEADBEEFU
#define MEMORY_FREED_MAGIC  0xAA00BB11U


// - - - Helper Functions - - -

static inline uint8_t* memoryTrackerGetPayload(MemoryHeader* HEADER)
{ return (uint8_t*)HEADER + sizeof(MemoryHeader); }

static inline MemoryHeader* memoryTrackerGetHeader(void* PAYLOAD)
{ return (MemoryHeader*) ((uint8_t*)PAYLOAD - sizeof(MemoryHeader)); }

static inline uint32_t* memoryTrackerGetFooter(MemoryHeader* HEADER)
{ return (uint32_t*)((uint8_t*)memoryTrackerGetPayload(HEADER) + HEADER->requestedSize); }


// - - - Add and remove from linked list - - -

static void memoryTrackerLinkNode(MemoryHeader* HEADER)
{
  HEADER->next = memoryActiveAllocations;
  HEADER->prev = NULL;
  if (memoryActiveAllocations != NULL)
  {
    memoryActiveAllocations->prev = HEADER;
  }
  memoryActiveAllocations = HEADER;

  memoryTagAllocatedBytes[HEADER->tag] += HEADER->requestedSize;
}

static void memoryTrackerUnlinkNode(MemoryHeader* HEADER)
{
  if (HEADER->prev != NULL)
  {
    HEADER->prev->next = HEADER->next;
  }
  else
  {
    memoryActiveAllocations = HEADER->next;
  }

  if (HEADER->next != NULL)
  {
    HEADER->next->prev = HEADER->prev;
  }

  memoryTagAllocatedBytes[HEADER->tag] -= HEADER->requestedSize;
}

// - - - Check for memory corruption
static bool memoryTrackerVerifyIntegrity(MemoryHeader* HEADER, const char* FILE, const char* FUNC, int32_t LINE)
{
  if (!HEADER) return false;

  if (HEADER->magic == MEMORY_FREED_MAGIC)
  {
    LOG_FATAL("[ENGINE MEMORY TRACKER] : Use-after-free or double-free detected at %s:%d in %s! Pointer was previously freed.",
              FILE, LINE, FUNC);
    abort();
  }

  if (HEADER->magic != MEMORY_HEADER_MAGIC)
  {
    LOG_FATAL("[ENGINE MEMORY TRACKER] : Header magic at %s:%d in %s! Expected 0x%X, got 0x%X",
              FILE, LINE, FUNC, MEMORY_HEADER_MAGIC, HEADER->magic);
    abort();
  }

  uint32_t* footer = memoryTrackerGetFooter(HEADER);
  uint32_t  footerVal;
  memcpy(&footerVal, footer, sizeof(uint32_t));

  if (footerVal != MEMORY_FOOTER_MAGIC)
  {
    LOG_FATAL("[ENGINE MEMORY TRACKER] : Buffer overflow detected! Tail magic overwritten for allocation of %zu bytes (origin: %s:%d in %s). Check triggered at %s:%d in %s.",
              HEADER->requestedSize, HEADER->file, HEADER->line, HEADER->func, FILE, LINE, FUNC);
    abort();
  }

  return true;
}


// - - - Engine API - - -

ENGINE_API void* memoryTrackedMalloc(
  size_t      SIZE,
  const char* FILE,
  const char* FUNC,
  int32_t     LINE,
  MemoryTag   TAG)
{
  ASSERT_DEBUG_MESSAGE(SIZE != 0, "[ENGINE MEMORY TRACKER] : Cannot allocate memory of SIZE 0");
  ASSERT_DEBUG_MESSAGE(TAG != MEMORY_TAG_COUNT, "[ENGINE MEMORY TRACKER] : Cannot allocate memory with invalid TAG");

  // - - - total size to malloc changes = HEADER + PAYLOAD + FOOTER
  size_t totalSize = sizeof(MemoryHeader) + SIZE + sizeof(uint32_t);

  if (totalSize + memoryTagAllocatedBytes[TAG] > memoryTagAllocationLimit[TAG])
  {
    LOG_FATAL("[ENGINE MEMORY TRACKER] : Limit (%zu) will be crossed by allocating %zu bytes at %s:%d in function %s for %s", 
              memoryTagAllocationLimit[TAG], SIZE, FILE, LINE, FUNC, memoryTagStrings[TAG]);
    return NULL;
  }

  MemoryHeader* header = (MemoryHeader*) malloc(totalSize);
  if (!header)
  {
    LOG_FATAL("[ENGINE MEMORY TRACKER] : malloc failure allocating %zu bytes at %s:%d in function %s for %s", 
              SIZE, FILE, LINE, FUNC, TAG);
    return NULL;
  }

  header->magic         = MEMORY_HEADER_MAGIC;
  header->tag           = TAG;
  header->file          = FILE;
  header->func          = FUNC;
  header->line          = LINE;
  header->requestedSize = SIZE;

  uint32_t footerMagic = MEMORY_FOOTER_MAGIC;
  memcpy(memoryTrackerGetFooter(header), &footerMagic, sizeof(uint32_t));

  memoryTrackerLinkNode(header);

  return memoryTrackerGetPayload(header);
}

ENGINE_API void* memoryTrackedCalloc(
  size_t      COUNT,
  size_t      SIZE,
  const char* FILE,
  const char* FUNC,
  int32_t     LINE,
  MemoryTag   TAG
)
{
  ASSERT_DEBUG_MESSAGE(COUNT  != 0, "[ENGINE MEMORY TRACKER] : Cannot callocate with a 0 COUNT");
  ASSERT_DEBUG_MESSAGE(SIZE   != 0, "[ENGINE MEMORY TRACKER] : Cannot callocate with a 0 SIZE");
  ASSERT_DEBUG_MESSAGE(TAG != MEMORY_TAG_COUNT, "[ENGINE MEMORY TRACKER] : Cannot callocate with invalid TAG");

  size_t  totalBytes  = COUNT * SIZE;
  void*   ptr         = memoryTrackedMalloc(totalBytes, FILE, FUNC, LINE, TAG);
  if (ptr)    memset(ptr, 0, totalBytes);

  return ptr;
}

ENGINE_API void* memoryTrackedRealloc(
  void*       PTR,
  size_t      NEW_SIZE,
  const char* FILE,
  const char* FUNC,
  int32_t     LINE
)
{
  ASSERT_DEBUG_MESSAGE(PTR != NULL, "[ENGINE MEMORY TRACKER] : Will not realloc NULL memory");
  ASSERT_DEBUG_MESSAGE(NEW_SIZE != 0, "[ENGINE MEMORY TRACKER] : Cannot reallocate with 0 NEW_SIZE");

  MemoryHeader* oldHeader = memoryTrackerGetHeader(PTR);
  if (!memoryTrackerVerifyIntegrity(oldHeader, FILE, FUNC, LINE))
  {
    return NULL;
  }

  memoryTrackerUnlinkNode(oldHeader);

  size_t        newTotalSize  = sizeof(MemoryHeader) + NEW_SIZE + sizeof(uint32_t);
  if (newTotalSize + memoryTagAllocatedBytes[oldHeader->tag] > memoryTagAllocationLimit[oldHeader->tag])
  {
    LOG_FATAL("[ENGINE MEMORY TRACKER] : Limit (%zu) will be crossed by allocating %zu bytes at %s:%d in function %s for %s", 
              memoryTagAllocationLimit[oldHeader->tag], NEW_SIZE, FILE, LINE, FUNC, oldHeader->tag);
    return NULL;
  }

  MemoryHeader* newHeader     = (MemoryHeader*) realloc(oldHeader, newTotalSize);

  if (!newHeader)
  {
    memoryTrackerLinkNode(oldHeader);
    LOG_FATAL("[ENGINE MEMORY TRACKER] : realloc failure reallocating %zu bytes at %s:%d in function %s for %s", 
              NEW_SIZE, FILE, LINE, FUNC, oldHeader->tag);
    return NULL;
  }

  newHeader->requestedSize  = NEW_SIZE;
  newHeader->file           = FILE;
  newHeader->func           = FUNC;
  newHeader->line           = LINE;

  uint32_t footerMagic = MEMORY_FOOTER_MAGIC;
  memcpy(memoryTrackerGetFooter(newHeader), &footerMagic, sizeof(uint32_t));

  memoryTrackerLinkNode(newHeader);
  return memoryTrackerGetPayload(newHeader);
}

ENGINE_API void memoryTrackedFree(
  void*       PTR,
  const char* FILE,
  const char* FUNC,
  int32_t     LINE)
{
  ASSERT_DEBUG_MESSAGE(PTR != NULL, "[ENGINE MEMORY TRACKER] : Cannot free a NULL PTR");

  MemoryHeader* header = memoryTrackerGetHeader(PTR);
  if (!memoryTrackerVerifyIntegrity(header, FILE, FUNC, LINE))
  { return; }

  memoryTrackerUnlinkNode(header);

  header->magic         = MEMORY_FREED_MAGIC;
  uint32_t freedFooter  = MEMORY_FREED_MAGIC;
  memcpy(memoryTrackerGetFooter(header), &freedFooter, sizeof(uint32_t));

  free(header);
}

ENGINE_API void memoryReportLeaks(void)
{
  if (memoryActiveAllocations == NULL)
  {
    LOG_INFO("[ENGINE MEMORY TRACKER] : No memory leaks detected. Clean exit.");
    return;
  }

  size_t totalLeakedBytes = 0;
  size_t leakCount        = 0;

  LOG_ERROR("[ENGINE MEMORY TRACKER] : Memory Leaks Detected, trying to report, but reporting itself may cause segfault");
  MemoryHeader* curr = memoryActiveAllocations;
  while (curr != NULL)
  {
    leakCount++;
    LOG_ERROR("[ENGINE MEMORY TRACKER] : Leak #%zu bytes | Tag: %s | Allocated at: %s:%d in function %s",
              leakCount, curr->requestedSize, memoryTagStrings[curr->tag], curr->file, curr->line, curr->func);

    totalLeakedBytes += curr->requestedSize;
    curr              = curr->next;
  }

  LOG_ERROR("[ENGINE MEMORY TRACKER] : Total Leaked : %zu bytes across %zu allocations.", totalLeakedBytes, leakCount);
}

ENGINE_API bool memoryCheckBounds(void)
{
  bool allClean = true;
  MemoryHeader* curr = memoryActiveAllocations;

  while (curr != NULL)
  {
    if (!memoryTrackerVerifyIntegrity(curr, __FILE__, __func__, __LINE__))
    { allClean = false; }
    curr = curr->next;
  }

  return allClean;
}

ENGINE_API size_t memoryGetLimit(MemoryTag TAG)
{
  ASSERT_DEBUG_MESSAGE(TAG != MEMORY_TAG_COUNT, "[ENGINE MEMORY TRACKER] : Cannot check limit of invalid TAG");
  return memoryTagAllocationLimit[TAG];
}

ENGINE_API void memorySetLimit(size_t LIMIT, MemoryTag TAG)
{
  ASSERT_DEBUG_MESSAGE(TAG != MEMORY_TAG_COUNT, "[ENGINE MEMORY TRACKER] : Cannot check limit of invalid TAG");

  if (memoryTagAllocatedBytes[TAG] >= LIMIT)
  {
    LOG_WARNING("[ENGINE MEMORY TRACKER] : New limit on %s is already crossed", memoryTagStrings[TAG]);
  }
  memoryTagAllocationLimit[TAG] = LIMIT;
}

ENGINE_API void memoryLogUsageStr(bool VERBOSE)
{
  #ifdef DEBUG
    const double gib = 1024.0 * 1024.0 * 1024.0;
    const double mib = 1024.0 * 1024.0;
    const double kib = 1024.0;

    // - - - Father allocation count per tag
    size_t tagAllocCount[MEMORY_TAG_COUNT] = {0};
    MemoryHeader* curr = memoryActiveAllocations;
    while (curr != NULL)
    {
      tagAllocCount[curr->tag]++;
      curr = curr->next;
    }

    LOG_INFO("[ENGINE MEMORY TRACKER] : System memory use (tagged):");

    // - - - Iterate through each tag
    for (uint8_t i = 0; i < MEMORY_TAG_COUNT; ++i)
    {
      size_t used  = memoryTagAllocatedBytes[i];
      size_t limit = memoryTagAllocationLimit[i];
      size_t ref   = (limit > used) ? limit : used;

      const char* unit    = "B";
      double      divisor = 1.0;

      if (ref >= (size_t)gib)
      {
        unit    = "GiB";
        divisor = gib;
      }
      else if (ref >= (size_t)mib)
      {
        unit    = "MiB";
        divisor = mib;
      }
      else if (ref >= (size_t)kib)
      {
        unit    = "KiB";
        divisor = kib;
      }

      double  usedScaled = (double)used / divisor;
      char    tagHeader[256];

      const char* special = (i + 1 == MEMORY_TAG_COUNT) ? "└──" : "├──";;

      if (limit > 0)
      {
        double limitScaled = (double)limit / divisor;
        double percentage  = ((double)used / (double)limit) * 100.0;

        snprintf(tagHeader, sizeof(tagHeader),
                "  %s %-14s: %.2f / %.2f %s (%.2f%%) [%zu alloc%s]",
                special, memoryTagStrings[i], usedScaled, limitScaled, unit, 
                percentage, tagAllocCount[i], tagAllocCount[i] == 1 ? "" : "s");
      }
      else
      {
        snprintf(tagHeader, sizeof(tagHeader),
                "  %s %-14s: %.2f %s (No Limit) [%zu alloc%s]",
                special, memoryTagStrings[i], usedScaled, unit,
                tagAllocCount[i], tagAllocCount[i] == 1 ? "" : "s");
      }

      LOG_INFO("%s", tagHeader);

      // - - - Verbose breakdown per allocation
      if (VERBOSE)
      {
        const char* bar = (i != MEMORY_TAG_COUNT - 1) ? "  │" : "   ";

        if (tagAllocCount[i] != 0)
        {
          MemoryHeader* alloc = memoryActiveAllocations;
          while (alloc != NULL)
          {
            if (alloc->tag == i)
            {
              // - - - Format size for individual entry
              const char* entryUnit       = "B";
              double      entryDivisor    = 1.0f;

              if (alloc->requestedSize >= (size_t) gib)
              {
                entryUnit     = "GiB";
                entryDivisor  = gib;
              }
              else if (alloc->requestedSize >= (size_t) mib)
              {
                entryUnit     = "MiB";
                entryDivisor  = mib;
              }
              else if (alloc->requestedSize >= (size_t) kib)
              {
                entryUnit     = "KiB";
                entryDivisor  = kib;
              }

              double entryScaled      = (double) alloc->requestedSize / entryDivisor;
              double entryPercentage  = ((double) alloc->requestedSize / (double) used) * 100.0;

              const char* special = (alloc->next == NULL || alloc->next->tag != alloc->tag) ? "└──" : "├──";
              LOG_DEBUG("%s  %s %s:%d in %s for %.2f %s (%.2f%% of the usage)",
                      bar, special, alloc->file, alloc->line, alloc->func,
                      entryScaled, entryUnit, entryPercentage);
            }
            alloc = alloc->next;
          }
        }
      }
    }
  #endif
}
