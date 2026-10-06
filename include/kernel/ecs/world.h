#pragma once

/**
 * @file : world.h
 * @brief : Defines common ecs types
 */

#include <kernel/justLibrary.h>
#include <stdint.h>
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

/// @brief : Iterator payload passed to system callbacks, this represents one archetype
typedef struct ecsSystemView
{
  size_t    entityCount;                      ///< Total active entities in current matching batch
  EntityID* entities;                         ///< Array of Entity handles
  void*     components[MAX_COMPONENT_COUNT];  ///< Raw pointers to requested components
} ECSSystemView;

/// @brief : function for a system to work on an archetype
typedef void (*ECSSystemCallback) (ECSSystemView* VIEW);

/// @brief : System representation
typedef struct ecsSystem
{
  uint64_t          requiredMask;       ///< Component bitmask required by system 
  ECSSystemCallback callback;           ///< The execution logic callback
  JustDynamicArray  matchedArchetypes;  ///< Pre-cached array of ArchetypeIDs
} ECSSystem;


// - - - | API | - - - 


// - - - System - - - 

/**
 * @brief : Creates and initializes a consolidated system with matching component dependencies
 * @param COMPONENT_COUNT : How many components does this system iterate over
 * @param COMPONENT_IDS : Array of the component ids, this system requires
 * @param CALLBACK : Iterate function
 * @return : An ecs system
 */
JUST_API ECSSystem ecsCreateSystem(
  size_t            COMPONENT_COUNT,
  ComponentID       COMPONENT_IDS[],
  ECSSystemCallback CALLBACK);

/**
 * @brief : EXecutes a single system across all matching entity archetypes
 * @param SYSTEM : The system to run
 */
JUST_API void ecsRunSystem(ECSSystem* SYSTEM);

/**
 * @brief : Destroys and cleans up a system
 * @param SYSTEM : The system to be destroyed
 */
JUST_API void ecsDestroySystem(ECSSystem* SYSTEM);


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

/**
 * @brief : Returns the archetype of a particular entity
 * @param ENTITY : Handle to the entity
 * @return : Handle to the archetype it belongs to
 */
JUST_API ArchetypeID ecsGetEntityArchetype(EntityID ENTITY);

/**
 * @brief : Changes the archetype of the entity
 * @param ENTITY : The entity whose archetype is to be changed
 * @param TARGET_ARCHETYPE : The new archetype
 */
JUST_API void ecsChangeArchetype(EntityID ENTITY, ArchetypeID TARGET_ARCHETYPE);


// - - - Component - - - 

/**
 * @brief : Used to register a component
 * @param SIZE : How big is the component type in bytes (including padding if any)
 * @warning : There is a limit to how many components can be registered
 * @see MAX_COMPONENT_COUNT : for how many components can be registered
 * @return : a handle to the component
 */
JUST_API ComponentID ecsRegisterComponent(size_t SIZE);

/// @brief : Easy macro access with type
#define ECS_REGISTER_COMPONENT(TYPE) ecsRegisterComponent(sizeof(TYPE))

/**
 * @brief : Tells whether a particular handle points to a registered component
 * @param COMPONENT : The component handle
 * @return : true if it is valid, false otherwise
 */
JUST_API static inline bool ecsIsComponentValid(ComponentID COMPONENT)
{  return COMPONENT != INVALID_COMPONENT;  }

/**
 * @brief : Returns a pointer to the component belonging to a particular entity
 * @param ENTITY : Handle to the entity
 * @param COMPONENT : Handle to the component type
 * @return : a void ptr to the component memory
 */
JUST_API void* ecsGetComponent(EntityID ENTITY, ComponentID COMPONENT);

/// @brief : Macro version with type handling
#define ECS_GET_COMPONENT(ENTITY, COMPONENT_ID, TYPE) \
  ((TYPE)* ecsGetComponent((ENTITY), (COMPONENT_ID)))


/**
 * @brief : Sets the value of a given entity's component
 * @param COMPONENT : Handle to the component type
 * @param DATA : Pointer to the data
 * @warning : Make sure data is valid
 */
JUST_API void ecsSetComponent(EntityID ENTITY, ComponentID COMPONENT, const void* DATA);

#define ECS_SET_COMPONENT(ENTITY, COMPONENT, VALUE) \
  ecsSetComponent((ENTITY), (COMPONENT), &VALUE)


/**
 * @brief : Tells whether a given entity has a particular type of component or not
 * @param ENTITY : Handle to the entity
 * @param COMPONENT : Handle to the component type
 * @return : true if it has it, false otherwise
 */
JUST_API static inline bool ecsHasComponent(EntityID ENTITY, ComponentID COMPONENT)
{ return ecsGetComponent(ENTITY, COMPONENT) != NULL; }

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
