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

/// @brief : Invalid component cannot exist
#define INVALID_COMPONENT (size_t)    SIZE_MAX

/// @brief : Invalid archetype cannot exist
#define INVALID_ARCHETYPE (uint64_t)  UINT64_MAX

/// @brief : Invalid entity cannot exist
#define INVALID_ENTITY    (uint64_t)  UINT64_MAX

/// @brief : Default archetype with 0 components
#define VOID_ARCHETYPE    (uint64_t)  0


// - - - | API | - - - 


// - - - World - - -

/**
 * @brief : Initializes the ecs world, allocates memory 
 * @warning : Must be called before any other ecs function that might modify something
 */
JUST_API void ecsInit(void);

/**
 * @brief : Shuts down the ecs world, frees memory
 * @warning : Must be called after any other ecs function that might modify something
 */
JUST_API void ecsShutdown(void);


// - - - Entity - - -


/**
 * @brief : Creates an entity of a specific archetype
 * @param ARCHETYPE : The id of the archetype
 * @warning : The Archetype must be registered first
 * @return : A handle to that entity 
 */
JUST_API EntityID ecsCreateEntity(ArchetypeID ARCHETYPE);

/**
 * @brief : Checks if a given entity is still valid
 * @param ENTITY : The handle of that entity
 * @return : true if valid, false otherwise
 */
JUST_API bool ecsIsEntityValid(EntityID ENTITY);



// - - - Component - - - 

/**
 * @brief : Used to register a component
 * @param SIZE : How big is the component type in bytes (including padding if any)
 * @see MAX_COMPONENT_COUNT : for how many components can be registered
 * @return : a handle to the component
 */
JUST_API ComponentID ecsRegisterComponent(size_t SIZE);

/**
 * @brief : Tells whether a particular handle points to a registered component
 * @param COMPONENT : The component handle
 * @return : true if it is valid, false otherwise
 */
JUST_API static inline bool ecsIsComponentValid(ComponentID COMPONENT)
{  return COMPONENT != INVALID_COMPONENT;  }


// - - - Archetype - - -

/**
 * @brief : Registers an archetype with the world
 * @param COMPONENT_COUNT : What is the size of the archetype, how many components does its entities hold
 * @param COMPONENT_IDS[] : What is the shape of the archetype, exactly what components does it's entities hold, an array of valid ComponentID handles
 * @return : A handle to the registered archetype
 */
JUST_API ArchetypeID ecsRegisterArchetype(size_t COMPONENT_COUNT, ComponentID COMPONENT_IDS[]);

/**
 * @brief : Tells whether a particular handle points to an archetype that has been registered
 * @param ARCHETYPE : THe handle to the archetype
 * @return : true if valid, false otherwise
 */
JUST_API static inline bool ecsIsArchetypeValid(ArchetypeID ARCHETYPE)
{
  return ARCHETYPE != INVALID_ARCHETYPE;
}
