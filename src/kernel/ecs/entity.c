/**
 * @file : ecs/entity.c
 * @brief : Handles entity functionality
 */

#include <kernel/ecs/world.h>
#include <kernel/ecs/_internal.h>
#include <stdint.h>
#include <string.h>

JUST_API EntityID ecsCreateEntity(ArchetypeID ARCHETYPE)
{
  ENSURE_AFTER_INIT
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
  ENSURE_AFTER_INIT

  JustDynamicArray* entityRegistry = &(_world->entityRegistry);

  // - - - Get the record from the entity id
  uint32_t      index   = ecsGetEntityIndex(ENTITY);
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
      justDynamicArrayPop(colArray, NULL);
    }

    EntityID* deadEntitySlot = (EntityID*) justDynamicArrayAt(&(arch->entityIds), deadRow);
    EntityID* lastEntitySlot = (EntityID*) justDynamicArrayAt(&(arch->entityIds), lastRow);
    *deadEntitySlot = *lastEntitySlot;
    justDynamicArrayPop(&arch->entityIds, NULL);

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
      justDynamicArrayPop(&(arch->components[col]), NULL);
    }
    justDynamicArrayPop(&(arch->entityIds), NULL);
  }
  arch->entityCount--;

  // - - - Invalidate records and free the entity
  record->archetypeID = INVALID_ARCHETYPE;
  record->generation++;
  justDynamicArrayPush(&(_world->freeEntityIndices), &index);
}

JUST_API bool ecsIsEntityValid(EntityID ENTITY)
{
  ENSURE_AFTER_INIT
  if (ENTITY == INVALID_ENTITY) return false;

  uint32_t index      = ecsGetEntityIndex(ENTITY);
  uint32_t generation = ecsGetEntityGeneration(ENTITY);

  if (index >= justDynamicArraySize(&(_world->entityRegistry))) return false;

  EntityRecord* record = (EntityRecord*) justDynamicArrayAt(&(_world->entityRegistry), index);
  return record->generation == generation && record->archetypeID != INVALID_ARCHETYPE;
}

JUST_API ArchetypeID ecsGetEntityArchetype(EntityID ENTITY)
{
  ENSURE_AFTER_INIT
  if (!ecsIsEntityValid(ENTITY)) return INVALID_ARCHETYPE;

  uint32_t      index   = ecsGetEntityIndex(ENTITY);
  EntityRecord* record  = (EntityRecord*) justDynamicArrayAt(&(_world->entityRegistry), index);
  return record->archetypeID;
}
