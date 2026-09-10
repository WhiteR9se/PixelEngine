#include <kernel/utils/logger.h>
#include <kernel/dataStructures/bitset.h>
#include <kernel/memory/tracker.h>

#define BITS_PER_WORD (sizeof(size_t) * 8)

static inline size_t bitsetGetTailMask(size_t CAPACITY)
{
  size_t remainder = CAPACITY % BITS_PER_WORD;
  if (remainder == 0) return ~(size_t)0;
  return ((size_t)1 << remainder) - 1;
}

ENGINE_API bool bitSetCreate(Bitset* BITSET, size_t CAPACITY, void* MEMORY)
{
  ASSERT_DEBUG_MESSAGE(BITSET != NULL, "[BISTET] : Cannot initialize a NULL BITSET");
  ASSERT_DEBUG_MESSAGE(CAPACITY > 0, "[BITSET[ : CAPACITY must be greater than 0");
  ASSERT_DEBUG_MESSAGE(((uintptr_t)MEMORY & (sizeof(size_t) - 1)) == 0, "[BITSET] : Provided MEMORY buffer is not properly aligned to word size");

  BITSET->capacity    = CAPACITY;
  BITSET->wordCount   = (CAPACITY + BITS_PER_WORD - 1) / BITS_PER_WORD;
  BITSET->ownsMemory  = (MEMORY == NULL);

  size_t totalBytes = BITSET->wordCount * sizeof(size_t);

  if (BITSET->ownsMemory)
  {
    BITSET->words = (size_t*) ENGINE_MALLOC(totalBytes, MEMORY_TAG_ARRAY);
    if (!BITSET->words)
    {
      LOG_ERROR("[BITSET] : Failed to allocate buffer of %zu bytes", totalBytes);
      return false;
    }
  }
  else
  {
    BITSET->words = (size_t*) MEMORY;
  }

  bitsetClearAll(BITSET);
  return true;
}

ENGINE_API void bitsetDestroy(Bitset* BITSET)
{
  ASSERT_DEBUG_MESSAGE(BITSET != NULL, "[BITSET] : Cannot destroy a NULL BITSET");

  if (BITSET->ownsMemory) 
  {
    ASSERT_DEBUG_MESSAGE(BITSET->words != NULL, "[BITSET] : Tried to destroy a corrupted bitset, words is NULL but set is not NULL and ownsMemory is true");
    ENGINE_FREE(BITSET->words);
  }

  BITSET->words       = NULL;
  BITSET->capacity    = 0;
  BITSET->wordCount   = 0;
  BITSET->ownsMemory  = false;
}

ENGINE_API bool bitsetEquals(const Bitset* A, const Bitset* B)
{
  ASSERT_DEBUG_MESSAGE(A != NULL && B != NULL, "[BITSET] : Cannot compute equals if operands are NULL");

  if (A->capacity != B->capacity)
  {
    LOG_WARNING("[BITSET] : Tried to check equals with diff capacity bitsets %p and %p", A, B);
    return false;
  }

  for (size_t i = 0; i < A->wordCount; ++i)
  {
    if (A->words[i] != B->words[i]) return false;
  }

  return true;
}

ENGINE_API void bitsetUnion(Bitset* DST, const Bitset* SRC)
{
  ASSERT_DEBUG_MESSAGE(DST != NULL, "[BISTET] : Cannot union with NULL DST");
  ASSERT_DEBUG_MESSAGE(SRC != NULL, "[BISTET] : Cannot union with NULL SRC");

  const size_t count = (DST->wordCount > SRC->wordCount) 
                        ? SRC->wordCount
                        : DST->wordCount;

  size_t* restrict dstWords = DST->words;
  size_t* restrict srcWords = SRC->words;

  for (size_t i = 0; i < count; ++i)
  { dstWords[i] |= srcWords[i]; }
}

ENGINE_API void bitsetIntersection(Bitset* DST, const Bitset* SRC)
{
  ASSERT_DEBUG_MESSAGE(DST != NULL, "[BISTET] : Cannot intersection with NULL DST");
  ASSERT_DEBUG_MESSAGE(SRC != NULL, "[BISTET] : Cannot intersection with NULL SRC");

  const size_t count = (DST->wordCount > SRC->wordCount) 
                        ? SRC->wordCount
                        : DST->wordCount;

  size_t* restrict dstWords = DST->words;
  size_t* restrict srcWords = SRC->words;

  for (size_t i = 0; i < count; ++i)
  { dstWords[i] &= srcWords[i]; }
}

ENGINE_API void bitsetDifference(Bitset* DST, const Bitset* SRC)
{
  ASSERT_DEBUG_MESSAGE(DST != NULL, "[BISTET] : Cannot difference with NULL DST");
  ASSERT_DEBUG_MESSAGE(SRC != NULL, "[BISTET] : Cannot difference with NULL SRC");

  const size_t count = (DST->wordCount > SRC->wordCount) 
                        ? SRC->wordCount
                        : DST->wordCount;

  size_t* restrict dstWords = DST->words;
  size_t* restrict srcWords = SRC->words;

  for (size_t i = 0; i < count; ++i)
  { dstWords[i] &= ~srcWords[i]; }
}

ENGINE_API void bitsetSet(Bitset* BITSET, size_t INDEX)
{
  ASSERT_DEBUG_MESSAGE(BITSET != NULL, "[BISTET] : Cannot set in a NULL BITSET");
  ASSERT_DEBUG_MESSAGE(INDEX < BITSET->capacity, "[BITSET] : Index out of bounds ");

  BITSET->words[INDEX / BITS_PER_WORD] |= ((size_t)1 << (INDEX % BITS_PER_WORD));
}

ENGINE_API void bitsetClear(Bitset* BITSET, size_t INDEX)
{
  ASSERT_DEBUG_MESSAGE(BITSET != NULL, "[BISTET] : Cannot set in a NULL BITSET");
  ASSERT_DEBUG_MESSAGE(INDEX < BITSET->capacity, "[BITSET] : Index out of bounds ");

  BITSET->words[INDEX / BITS_PER_WORD] &= ~((size_t)1 << (INDEX % BITS_PER_WORD));
}

ENGINE_API void bitsetToggle(Bitset* BITSET, size_t INDEX)
{
  ASSERT_DEBUG_MESSAGE(BITSET != NULL, "[BISTET] : Cannot set in a NULL BITSET");
  ASSERT_DEBUG_MESSAGE(INDEX < BITSET->capacity, "[BITSET] : Index out of bounds ");

  BITSET->words[INDEX / BITS_PER_WORD] ^= ((size_t)1 << (INDEX % BITS_PER_WORD));
}

ENGINE_API bool bitsetGet(Bitset* BITSET, size_t INDEX)
{
  ASSERT_DEBUG_MESSAGE(BITSET != NULL, "[BISTET] : Cannot set in a NULL BITSET");
  ASSERT_DEBUG_MESSAGE(INDEX < BITSET->capacity, "[BITSET] : Index out of bounds ");

  return (BITSET->words[INDEX / BITS_PER_WORD] & (size_t)1 << (INDEX % BITS_PER_WORD));
}

ENGINE_API void bitsetClearAll(Bitset* BITSET)
{
  ASSERT_DEBUG_MESSAGE(BITSET != NULL, "[BISTET] : Cannot set in a NULL BITSET");

  memset(BITSET->words, 0, BITSET->wordCount * sizeof(size_t));
}

ENGINE_API void bitsetSetAll(Bitset* BITSET)
{
  ASSERT_DEBUG_MESSAGE(BITSET != NULL, "[BISTET] : Cannot set in a NULL BITSET");

  memset(BITSET->words, 0xFF, BITSET->wordCount * sizeof(size_t));
  BITSET->words[BITSET->wordCount - 1] &= bitsetGetTailMask(BITSET->capacity);
}
