#pragma once

/**
 * @file : world.h
 * @brief : Defines common ecs types
 */

#include <kernel/justLibrary.h>
#define MAX_COMPONENT_COUNT 64


// - - - Types - - -

/// @brief : ComponentID is just an index to the bit given to it in the archetype
typedef size_t    ComponentID;

/// @brief : ArchetypeID is a bitmask for the archetype
typedef uint64_t  ArchetypeID;

/// @brief : Entity ID is a 64 bit int, which is divided into 32 bits (index) and 32 bits (generation)
typedef uint64_t  EntityID;

#define INVALID_COMPONENT (size_t)    SIZE_MAX
#define INVALID_ARCHETYPE (uint64_t)  UINT64_MAX
#define INVALID_ENTITY    (uint64_t)  UINT64_MAX


// - - - | API | - - - 


// - - - World - - -

JUST_API void ecsInit(void);

JUST_API void ecsShutdown(void);


// - - - Entity - - -

JUST_API EntityID ecsCreateEntity(ArchetypeID ARCHETYPE);

JUST_API static inline bool ecsIsEntityValid(EntityID ENTITY)
{ 
  return ENTITY != INVALID_ENTITY; 
}

static inline EntityID ecsMakeEntity(uint32_t INDEX, uint32_t GENERATION)
{
  return ((uint64_t) GENERATION << 32) | (uint32_t) INDEX;
}

static inline uint32_t ecsGetEntityIndex(EntityID ENTITY)
{
  return (uint32_t) (ENTITY & 0xFFFFFFFF);
}

static inline uint32_t ecsGetEntityGeneration(EntityID ENTITY)
{
  return (uint32_t) (ENTITY >> 32);
}


// - - - Component - - - 

JUST_API ComponentID ecsRegisterComponent(size_t SIZE);

JUST_API static inline bool ecsIsComponentValid(ComponentID COMPONENT)
{
  return COMPONENT != INVALID_COMPONENT; 
}


// - - - Archetype - - -

JUST_API ArchetypeID ecsRegisterArchetype(size_t COMPONENT_COUNT, ComponentID COMPONENT_IDS[]);

JUST_API static inline bool ecsIsArchetypeValid(ArchetypeID ARCHETYPE)
{
  return ARCHETYPE != INVALID_ARCHETYPE;
}
