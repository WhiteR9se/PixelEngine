/**
 * @file : ecs/world.c
 * @brief : Handles world initialize destroy and other functions
 */

#include <kernel/ecs/_internal.h>
#include <stddef.h>
#include <limits.h>
#include <stdint.h>


static const char* worldTag = "ECS";
ECSWorld* _world = NULL;

JUST_API void ecsInit(void)
{
  JUST_LOG_DEBUG("Trying to initialize ECS");

  // - - - check initialize once
  JUST_ASSERT_DEBUG_MESSAGE(!ecsIsWorldValid(), "[ECS] : Trying to double initialize ECS");

  // - - - remove memory limit
  size_t noLimit = 0;
  justMemorySetLimit(noLimit, worldTag);

  // - - - allocate memory for the world 
  size_t memNeeded = sizeof(ECSWorld) + (MAX_COMPONENT_COUNT * sizeof(ComponentInfo));
  _world = JUST_MALLOC_TAGGED(memNeeded, worldTag);
  JUST_ASSERT_DEBUG_MESSAGE(ecsIsWorldValid(), "[ECS] : Failed to allocate enough memory ");

  // - - - allocate memory for archetypeRegistry
  bool ok = JUST_DARRAY_INIT_TAGGED(&(_world->archetypeRegistry), 0, Archetype, worldTag);
  JUST_ASSERT_DEBUG_MESSAGE(ok, "[ECS] : Could not initialize archetype registry");

  // - - - allocate memory for entity registry
  ok = JUST_DARRAY_INIT_TAGGED(&(_world->entityRegistry), 0, EntityRecord, worldTag);
  JUST_ASSERT_DEBUG_MESSAGE(ok, "[ECS] : Failed to init entity records");

  // - - - allocate memory for free indices
  ok = JUST_DARRAY_INIT_TAGGED(&(_world->freeEntityIndices), 0, uint32_t, worldTag);
  JUST_ASSERT_DEBUG_MESSAGE(ok, "[ECS] : Failed to init free indices stack");

  // - - - initialize component registry
  for (uint8_t i = 0; i < MAX_COMPONENT_COUNT; ++i) _world->componentRegistry[i] = SIZE_MAX;
  _world->componentIndex = 0;
  JUST_LOG_TRACE("[ECS] : Initialized component registry");

  // - - - create the void archetype
  ArchetypeID voidID = ecsRegisterArchetype(0, NULL);
  JUST_ASSERT_DEBUG_MESSAGE(voidID == VOID_ARCHETYPE, "[ECS] : Failed to register VOID_ARCHETYPE");

  JUST_LOG_INFO("[ECS] : Initialized world");
}

JUST_API void ecsShutdown(void)
{
  JUST_LOG_DEBUG("[ECS] : Shutting down");

  JUST_ASSERT_DEBUG_MESSAGE(ecsIsWorldValid(), "[ECS] : Trying to double shutdown or shutting down an unitialized ECS");

  // - - - Clear all the archetypes
  JustDynamicArray* archRegistry  = &(_world->archetypeRegistry);
  Archetype*        archs         = JUST_DARRAY_DATA(archRegistry, Archetype);

  for (size_t archIndex = 0; archIndex < justDynamicArraySize(archRegistry); ++archIndex)
  {
    Archetype arch = archs[archIndex];

    // - - - Destroy all its dyanmic arrays
    for (uint8_t compIndex = 0; compIndex < arch.componentCount; ++compIndex)
    {
      JustDynamicArray* componentMemory = &(arch.components[compIndex]);
      justDynamicArrayDestroy(componentMemory);
    }

    // - - - Free all the component pointers
    JUST_FREE(arch.components);
  }
  justDynamicArrayDestroy(archRegistry);

  // - - - clear out the entity registry
  justDynamicArrayDestroy(&(_world->entityRegistry));
  justDynamicArrayDestroy(&(_world->freeEntityIndices));

  // - - - Clear out the component registy
  _world->componentIndex = 0;

  // - - - Destroy the world
  JUST_FREE(_world);
  _world = NULL;

  JUST_LOG_INFO("[ECS] : Shut down");
}
