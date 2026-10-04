/**
 * @file : ecs/archetype.c
 * @brief : Handles archetype related functionality
 */

#include <kernel/ecs/_internal.h>
#include <stdio.h>

Archetype* ecsInternalGetArchetype(ArchetypeID ID)
{
  ENSURE_AFTER_INIT
  return &JUST_DARRAY_GET(&(_world->archetypeRegistry), Archetype, ID); 
}

JUST_API ArchetypeID ecsRegisterArchetype(size_t COMPONENT_COUNT, ComponentID COMPONENT_IDS[])
{
  ENSURE_AFTER_INIT

  // - - - Create component mask
  uint64_t mask = 0;
  for (size_t i = 0; i < COMPONENT_COUNT; ++i)
  {
    JUST_ASSERT_DEBUG_MESSAGE(COMPONENT_IDS[i] < _world->componentIndex, "[ECS] : Invalid COMPONENT_IDS");
    mask |= ((uint64_t)1 << COMPONENT_IDS[i]);
  }

  JustDynamicArray* archRegistry      = &(_world->archetypeRegistry);
  
  //- - - Check if archetype is already registered
  Archetype*        archs             = JUST_DARRAY_DATA(archRegistry, Archetype);
  size_t            currentArchCount  = justDynamicArraySize(archRegistry);
  for (size_t i = 0; i < currentArchCount; ++i)
  {
    JUST_ASSERT_DEBUG_MESSAGE(archs[i].mask != mask, "[ECS] : Archetype already registered");
  }

  // - - - Create the new archetype
  Archetype*  newArch     = JUST_DARRAY_EMPLACE(archRegistry, Archetype);
  ArchetypeID id          = justDynamicArraySize(archRegistry) - 1;
  newArch->mask           = mask;
  newArch->entityCount    = 0;
  newArch->componentCount = COMPONENT_COUNT;

  // - - - Invalidate the entire component to column mapping
  memset(newArch->compIdToColumnMap, INVALID_MAPPING, sizeof(newArch->compIdToColumnMap));

  // - - - Allocate enough memory for component storage
  char tag[32];
  snprintf(tag, sizeof(tag), "Arch_%zu", id);

  if (COMPONENT_COUNT > 0)
  {
    size_t sizeReq      = sizeof(JustDynamicArray) * COMPONENT_COUNT;
    newArch->components = JUST_MALLOC_TAGGED(sizeReq, tag);

    // - - - make the mapping again
    for (size_t i = 0; i < COMPONENT_COUNT; ++i)
    {
      ComponentID compId                  = COMPONENT_IDS[i];
      newArch->compIdToColumnMap[compId]  = (uint8_t)i;
      size_t compSize                     = _world->componentRegistry[compId];

      bool ok = justDynamicArrayCreate(&(newArch->components[i]), 0, compSize, NULL, tag);
      JUST_ASSERT_DEBUG_MESSAGE(ok, "[ECS] : Failed to create column array");
    }
  }
  else newArch->components = NULL;

  // - - - Get entitites ready
  bool ok = justDynamicArrayCreate(&(newArch->entityIds), 0, sizeof(EntityID), NULL, tag);
  JUST_ASSERT_DEBUG_MESSAGE(ok, "[ECS] : Failed to create entity ID array");

  return id;
}
