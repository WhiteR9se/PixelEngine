#include <kernel/ecs/world.h>
#include <kernel/ecs/_internal.h>
#include <stdint.h>

JUST_API EntityID ecsCreateEntity(ArchetypeID ARCHETYPE)
{
  JUST_ASSERT_DEBUG_MESSAGE(ARCHETYPE < justDynamicArraySize(&(_world->archetypeRegistry)), "[ECS] : Invalid ArchetypeID");

  Archetype* arch = ecsInternalGetArchetype(ARCHETYPE);
  uint32_t index;
  uint32_t generation;

  if (!justDynamicArrayIsEmpty(&(_world->freeEntityIndices)))
  {
    justDynamicArrayPop(&(_world->freeEntityIndices), &index);
    EntityRecord* record = (EntityRecord*) justDynamicArrayAt(&(_world->entityRegistry), index);

  

    
  }
}
