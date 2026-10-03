#include <kernel/ecs/world.h>
#include <kernel/ecs/_internal.h>
#include <stdint.h>

JUST_API EntityID ecsCreateEntity(ArchetypeID ARCHETYPE)
{
  JUST_ASSERT_DEBUG_MESSAGE(ARCHETYPE < justDynamicArraySize(&(_world->archetypeRegistry)), "[ECS] : Invalid ArchetypeID");

  Archetype*        arch            = ecsInternalGetArchetype(ARCHETYPE);
  JustDynamicArray* entityRegistry  = &(_world->entityRegistry);
  JustDynamicArray* freeEntities    = &(_world->freeEntityIndices);

  uint32_t index;
  uint32_t generation;

  // - - - If free entities are available, use that
  if (!justDynamicArrayIsEmpty(freeEntities))
  {
    justDynamicArrayPop(freeEntities, &index);
    EntityRecord* record  = (EntityRecord*) justDynamicArrayAt(entityRegistry, index);
    generation            = record->generation;  
  }

  // - - - Otherwise create a new entity
  else
  {
    EntityRecord* record  = JUST_DARRAY_EMPLACE(entityRegistry, EntityRecord);
    index                 = (uint32_t) (justDynamicArraySize(entityRegistry) - 1);
    generation            = 1;
    record->generation    = generation;
  }

  // - - - Make its record
  uint32_t      newRow  = (uint32_t) arch->entityCount;
  EntityRecord* record  = (EntityRecord*) justDynamicArrayAt(entityRegistry, index);
  record->archetypeID   = ARCHETYPE;
  record->rowIndex      = newRow;

  // - - - Add it to the component memory of the archetype
  for (size_t col = 0; col < arch->componentCount; ++col)
  {
    justDynamicArrayEmplace(&(arch->components[col]));
  }

  // - - - Create the entity id
  EntityID handle = ecsMakeEntity(index, generation);
  justDynamicArrayPush(&(arch->entityIds), &handle);
  arch->entityCount++;

  return handle;
}

JUST_API void ecsDestroyEntity(EntityID ENTITY)
{
  JUST_ASSERT_DEBUG_MESSAGE(ecsIsEntityValid(ENTITY), "[ECS] : Attempted to destroy an Invalid ENTITY");

  JustDynamicArray* entityRegistry = &(_world->entityRegistry);

  // - - - Get the record from the entity id
  uint32_t      index   = ecsEntityGetIndex(ENTITY);
  EntityRecord* record  = (EntityRecord*) justDynamicArrayAt(entityRegistry, index);

  // - - - Get Archetype data from the record
  Archetype*  arch    = ecsInternalGetArchetype(record->archetypeID);
  uint32_t    deadRow = record->rowIndex;
  uint32_t    lastRow = (uint32_t)(arch->entityCount - 1);

  // - - - swap and pop if in the middle
  if (deadRow != lastRow)
  {
    // - - - swap components
    for (size_t col = 0; col < arch->componentCount; ++col)
    {
      JustDynamicArray* colArray  = &(arch->components[col]);
      void* deadSlot              = justDynamicArrayAt(colArray, deadRow);
      void* lastSlot              = justDynamicArrayAt(colArray, lastRow);

      memcpy(deadSlot, lastSlot, colArray->elementSize);
      colArray->size--;
    }

    EntityID* deadEntitySlot = (EntityID*) justDynamicArrayAt(entityRegistry, deadRow);
    EntityID* lastEntitySlot = (EntityID*) justDynamicArrayAt(entityRegistry, lastRow);
    *deadEntitySlot = *lastEntitySlot;
    arch->entityIds.size--;

    EntityID      movedEntity = *deadEntitySlot;
    uint32_t      movedIndex  = ecsGetEntityIndex(movedEntity);
    EntityRecord* movedRecord = (EntityRecord*) justDynamicArrayAt(entityRegistry, movedIndex);
    movedRecord->rowIndex     = deadRow;
  }

  // - - - otherwise, it is easy, just reduce size
  else
  {
    for (size_t col = 0; col < arch->componentCount; ++col)
    {
      arch->components[col].size--;
    }
    arch->entityIds.size--;
  }

  arch->entityCount--;

  // - - - Invalidate records and free the entity
  record->archetypeID = INVALID_ARCHETYPE;
  record->generation++;
  justDynamicArrayPush(&(_world->freeEntityIndices), &index);
}