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
  EntityID    entityID;     ///< What entity
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

// - - - Global private instance
extern ECSWorld* _world;

// - - - Internal helper declarations
bool        ecsIsWorldValid(void);
Archetype*  ecsInternalGetArchetype(ArchetypeID ARCHETYPE);
