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

#define INVALID_COMPONENT (size_t)    SIZE_MAX
#define INVALID_ARCHETYPE (uint64_t)  UINT64_MAX


// - - - Archetype API - - - 

JUST_API void ecsInit(void);
JUST_API void ecsShutdown(void);

JUST_API ComponentID ecsRegisterComponent(size_t SIZE);

JUST_API ArchetypeID ecsRegisterArchetype(size_t COMPONENT_COUNT, ComponentID COMPONENT_IDS[]);
