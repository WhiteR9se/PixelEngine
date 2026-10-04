#pragma once

/**
 * @file : ecs/_internal.h
 * @brief : Provides common points and types for ecs
 * @warning : This file is not be included except by the engine, if you are a  game developer, please do not include this
*/

#include <kernel/ecs/world.h>
#include <stdint.h>

#define INVALID_MAPPING 0xFF

/// @brief : All we need for a component is its size
typedef size_t ComponentInfo;

/// @brief : Internal record mapping an entity index to its archetype location
typedef struct entityRecord
{
  ArchetypeID archetypeID;  ///< What archetype does this entity belong to
  uint32_t    rowIndex;     ///< Index
  uint32_t    generation;   ///< Generation
} EntityRecord;

/// @brief : Archetype metadata 
typedef struct archetype
{
  uint64_t          mask;                                   ///< Mask, representing what components does this store
  size_t            entityCount;                            ///< How many entities does this store
  size_t            componentCount;                         ///< How many components worth
  JustDynamicArray* components;                             ///< Component memory
  uint8_t           compIdToColumnMap[MAX_COMPONENT_COUNT]; ///< Mapping of component ids to component array indexes
  JustDynamicArray  entityIds;                              ///< Entity storage
} Archetype;

/// @brief : Global state of the ECS
typedef struct ecsWorld
{
  JustDynamicArray  archetypeRegistry;                      ///< Stores archetypes
  JustDynamicArray  entityRegistry;                          ///< Stores entity records
  JustDynamicArray  freeEntityIndices;                      ///< Entities previously freed, so can be used again.
  size_t            componentIndex;                         ///< How many components
  ComponentInfo     componentRegistry[MAX_COMPONENT_COUNT]; ///< Component storage
} ECSWorld;

/// @brief: Global private instance
extern ECSWorld* _world;

// - - - Internal helper declarations - - -

/**
 * @brief : Tells whether the world has been initialized or not
 * @return : true if the world has been initialized, false otherwise
 */
static inline bool ecsIsWorldValid(void) 
{ return _world != NULL; }

#define ENSURE_AFTER_INIT JUST_ASSERT_DEBUG_MESSAGE(ecsIsWorldValid(), "[ECS] : World not initialized!");

/**
 * @brief : Gets archetype from id
 * @param ARCHETYPE : The Archetype id
 * @return : Pointer to the refered archhetype
 */
Archetype* ecsInternalGetArchetype(ArchetypeID ARCHETYPE);

/**
 * @brief : creeats an entity handle out of index and generation
 * @param INDEX : The row id of the entity in its archetype
 * @param GENERATION : The amount of time, this slot has been reused.
 * @return : A constructed handle
 */
static inline EntityID ecsMakeEntity(uint32_t INDEX, uint32_t GENERATION)
{
  return ((uint64_t) GENERATION << 32) | (uint32_t) INDEX;
}

/**
 * @brief : extracts the index from an entity handle
 * @param ENTITY : The entity handle
 * @return : the row id index in its archetype
 */
static inline uint32_t ecsGetEntityIndex(EntityID ENTITY)
{
  return (uint32_t) (ENTITY & 0xFFFFFFFF);
}

/**
 * @brief : extracts the generation from an entity handle
 * @param ENTITY : The entity handle
 * @return : the amount of time this handle has been recycled
 */
static inline uint32_t ecsGetEntityGeneration(EntityID ENTITY)
{
  return (uint32_t) (ENTITY >> 32);
}
