#include <kernel/ecs/_internal.h>
#include <kernel/ecs/world.h>
#include <stdint.h>

JUST_API ECSSystem ecsRegisterSystem(
  size_t            COMPONENT_COUNT, 
  ComponentID       COMPONENT_IDS[], 
  ECSSystemCallback CALLBACK)
{
  ENSURE_AFTER_INIT
  JUST_ASSERT_DEBUG_MESSAGE(CALLBACK != NULL, "[ECS] : System CALLBACK cannot be NULL");

  ECSSystem system;
  system.callback     = CALLBACK;
  system.requiredMask = 0;

  // - - - Build requirement bitmask
  for (size_t i = 0; i < COMPONENT_COUNT; ++i)
  {
    JUST_ASSERT_DEBUG_MESSAGE(ecsIsComponentValid(COMPONENT_IDS[i]), "[ECS] : Invalid COMPONENT_IDS"); 
    system.requiredMask |= ((uint64_t)1 << COMPONENT_IDS[i]);
  }

  // - - - Initialize archetype cache
  bool ok = JUST_DARRAY_INIT_TAGGED(&(system.matchedArchetypes), 0, ArchetypeID, "ECS_SYSTEMS");
  JUST_ASSERT_DEBUG_MESSAGE(ok, "[ECS] : Failed to allocate system archetype cache");

  // - - - fill the cache
  JustDynamicArray* archRegistry  = &(_world->archetypeRegistry);
  Archetype*        archs         = JUST_DARRAY_DATA(archRegistry, Archetype);
  size_t            archCount     = justDynamicArraySize(archRegistry);

  for (size_t i = 0; i < archCount; ++i)
  {
    if ((archs[i].mask & system.requiredMask) == system.requiredMask)
    {
      justDynamicArrayPush(&(system.matchedArchetypes), &i);
    }
  }

  return system;
}

JUST_API void ecsRunSystem(ECSSystem* SYSTEM)
{
  JUST_ASSERT_DEBUG_MESSAGE(SYSTEM != NULL, "[ECS] : Cannot run a NULL system");
  JUST_ASSERT_DEBUG_MESSAGE(SYSTEM->callback != NULL, "[ECS] : System callback is NULL");

  size_t        matchedCount  = justDynamicArraySize(&(SYSTEM->matchedArchetypes));
  ArchetypeID*  matchedIDs    = JUST_DARRAY_DATA(&(SYSTEM->matchedArchetypes), ArchetypeID);

  for (size_t i = 0; i < matchedCount; ++i)
  {
    Archetype* arch = ecsInternalGetArchetype(matchedIDs[i]);
    if (arch->entityCount == 0) continue;

    ECSSystemView view;
    view.entityCount  = arch->entityCount;
    view.entities     = JUST_DARRAY_DATA(&(arch->entityIds), EntityID);

    // - - - Populate component array pointers for components requested by mask
    for (size_t compID = 0; compID < _world->componentIndex; ++compID)
    {
      if (SYSTEM->requiredMask & ((uint64_t)1 << compID))
      {
        uint8_t col = arch->compIdToColumnMap[compID];
        view.components[compID] = JUST_DARRAY_DATA(&arch->components[col], void*);
      }
      else
      {
        view.components[compID] = NULL;
      }
    }

    // - - - Execute on the memory chunk
    SYSTEM->callback(&view);
  }
}

JUST_API void ecsDestroySystem(ECSSystem* SYSTEM)
{
  JUST_ASSERT_DEBUG_MESSAGE(SYSTEM != NULL, "[ECS] : Cannot destroy a NULL SYSTEM");
  justDynamicArrayDestroy(&(SYSTEM->matchedArchetypes));
}
