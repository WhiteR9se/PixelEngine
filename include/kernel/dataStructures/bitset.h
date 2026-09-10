/**
 * @brief : Lightweight dynamic bitset
*/

#include <kernel/defines.h>
#include <kernel/utils/assert.h>
#include <memory.h>
#include <stddef.h>

#ifdef __cplusplus
  extern "C" {
#endif

#define BITS_PER_WORD (sizeof(size_t) * 8)

/// @brief : Bitset struct
typedef struct bitset
{
  size_t  capacity;   ///< Total trackable entity count
  size_t  wordCount;  ///< Number of 64-bit backing words
  size_t* words;      ///< Backing memory buffer
  bool    ownsMemory; ///< True if allocated internally, false if passed in
} Bitset;

inline size_t bitsetGetTailMask(size_t CAPACITY)
{
  size_t remainder = CAPACITY % BITS_PER_WORD;
  if (remainder == 0) return ~(size_t)0;
  return ((size_t)1 << remainder) - 1;
}

/**
 * @brief : Initializes a bitset
 * @param SET : Pointer to the bitset struct to initialize.
 * @param CAPACITY : Number of bits / entities to track
 * @param MEMORY : Optional pre-allocaed memory buffer or NULL to allocate
 * @warning : MEMORY must be aligned to sizeof(size_t)
 * @return : true on success, false on failure
*/
ENGINE_API bool bitSetCreate(Bitset* BITSET, size_t CAPACITY, void* MEMORY);

/**
 * @brief : Destroys the bitset and frees backing memory if owned
 * @param BITSET: Pointer to the bitset to destroy
*/
ENGINE_API void bitsetDestroy(Bitset* BITSET);

/**
 * @brief : Compares two bitsets for equality
 * @param A : One bitset
 * @param B : Second bitset
 * @return : True if identical in capacity and set bits, false otherwise
 */
ENGINE_API bool bitsetEquals(const Bitset* A, const Bitset* B);

/**
 * @brief : Sets a bit at index to 1
 * @param BITSET : Pointer to the bitset
 * @param INDEX : Which index to set
*/
ENGINE_API inline void bitsetSet(Bitset* BITSET, size_t INDEX)
{
  ASSERT_DEBUG_MESSAGE(BITSET != NULL, "[BISTET] : Cannot set in a NULL BITSET");
  ASSERT_DEBUG_MESSAGE(INDEX < BITSET->capacity, "[BITSET] : Index out of bounds ");

  BITSET->words[INDEX / BITS_PER_WORD] |= ((size_t)1 << (INDEX % BITS_PER_WORD));
}

/**
 * @brief : Sets a bit at index to 0
 * @param BITSET : Pointer to the bitset
 * @param INDEX : Which index to clear
*/
ENGINE_API inline void bitsetClear(Bitset* BITSET, size_t INDEX)
{
  ASSERT_DEBUG_MESSAGE(BITSET != NULL, "[BISTET] : Cannot set in a NULL BITSET");
  ASSERT_DEBUG_MESSAGE(INDEX < BITSET->capacity, "[BITSET] : Index out of bounds ");

  BITSET->words[INDEX / BITS_PER_WORD] &= ~((size_t)1 << (INDEX % BITS_PER_WORD));
}

/**
 * @brief : Toggles a bit at index
 * @param BITSET : Pointer to the bitset
 * @param INDEX : Which index to toggle
*/
ENGINE_API inline void bitsetToggle(Bitset* BITSET, size_t INDEX)
{
  ASSERT_DEBUG_MESSAGE(BITSET != NULL, "[BISTET] : Cannot set in a NULL BITSET");
  ASSERT_DEBUG_MESSAGE(INDEX < BITSET->capacity, "[BITSET] : Index out of bounds ");

  BITSET->words[INDEX / BITS_PER_WORD] ^= ((size_t)1 << (INDEX % BITS_PER_WORD));
}

/**
 * @brief : Gets a bit at index
 * @param BITSET : Pointer to the bitset
 * @param INDEX : Which index to set
 * @return : True if the bit is set, false otherwise
*/
ENGINE_API inline bool bitsetGet(Bitset* BITSET, size_t INDEX)
{
  ASSERT_DEBUG_MESSAGE(BITSET != NULL, "[BISTET] : Cannot set in a NULL BITSET");
  ASSERT_DEBUG_MESSAGE(INDEX < BITSET->capacity, "[BITSET] : Index out of bounds ");

  return (BITSET->words[INDEX / BITS_PER_WORD] & (size_t)1 << (INDEX % BITS_PER_WORD));
}

/**
 * @brief : Clears all bits to 0
 * @param BITSET : Pointer to the bitset
*/
ENGINE_API inline void bitsetClearAll(Bitset* BITSET)
{
  ASSERT_DEBUG_MESSAGE(BITSET != NULL, "[BISTET] : Cannot set in a NULL BITSET");

  memset(BITSET->words, 0, BITSET->wordCount * sizeof(size_t));
}

/**
 * @brief : Sets all bits to 1
 * @param BITSET : Pointer to the bitset
*/
ENGINE_API inline void bitsetSetAll(Bitset* BITSET)
{
  ASSERT_DEBUG_MESSAGE(BITSET != NULL, "[BISTET] : Cannot set in a NULL BITSET");

  memset(BITSET->words, 0xFF, BITSET->wordCount * sizeof(size_t));
  BITSET->words[BITSET->wordCount - 1] &= bitsetGetTailMask(BITSET->capacity);
}

/**
 * @brief : Computes in -place union : DST = DST | SRC
 * @warning : The smaller capacity is used
 * @warning : There should be no memory overlap
 * @param DST : The destination bitset
 * @param SRC : The source bitset
*/
ENGINE_API void bitsetUnion(Bitset* DST, const Bitset* SRC);

/**
 * @brief : Computes in -place intersection : DST = DST & SRC
 * @warning : The smaller capacity is used
 * @warning : There should be no memory overlap
 * @param DST : The destination bitset
 * @param SRC : The source bitset
*/
ENGINE_API void bitsetIntersection(Bitset* DST, const Bitset* SRC);

/**
 * @brief : Computes in -place difference : DST = DST & ~SRC
 * @warning : The smaller capacity is used
 * @warning : There should be no memory overlap
 * @param DST : The destination bitset
 * @param SRC : The source bitset
*/
ENGINE_API void bitsetDifference(Bitset* DST, const Bitset* SRC);
