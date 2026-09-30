#pragma once

/**
 * @file : world.h
 * @brief : Defines common ecs types
 */

#include <kernel/justLibrary.h>
#define MAX_COMPONENT_COUNT 64

// - - - Types - - -

/// @brief : EntityID is just an index to entities
typedef size_t    EntityID;

/// @brief : ComponentID is just an index to the bit given to it in the archetype
typedef size_t    ComponentID;

/// @brief : ArchetypeID is a bitmask for the archetype
typedef uint64_t  ArchetypeID;

/// @brief : Defines an entity
typedef struct entity
{
  EntityID  id;           ///< The id, the index to be used everywhere 
  size_t    generation;   ///< Just counts how many types has it been used to manage recycling
} Entity;

/// @brief : defines an archetype
typedef struct archetype
{
  ArchetypeID mask;           ///< The bitmask of an archetype
  size_t      entityCount;    ///< How many entities of this archectype 
  size_t      entityCapacity; ///< How many entities can this archetype hold rn
  size_t      componentCount; ///< How many components do these entites have
} Archetype;


// - - - Archetype API - - - 

JUST_API void ecsInit(void);
JUST_API void ecsShutdown(void);

JUST_API size_t ecsRegisterComponent(size_t SIZE);

JUST_API size_t ecsRegisterArchetype(size_t COMPONENT_COUNT, ComponentID COMPONENT_IDS[]);

JUST_API bool   ecsAddEntity(ArchetypeID ARCHETYPE, EntityID Entity);
