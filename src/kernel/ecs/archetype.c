/**
 * @file : ecs/archetype.c
 * @brief : Handles archetype related functionality
 */

#include "kernel/ecs/world.h"
#include <kernel/ecs/_internal.h>
#include <stddef.h>
#include <stdint.h>
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
  char archTag[32];
  snprintf(archTag, sizeof(archTag), "ECS_Arch_%zu", id);

  if (COMPONENT_COUNT > 0)
  {
    size_t sizeReq  = sizeof(JustDynamicArray) * COMPONENT_COUNT;
    size_t limit    = 1024 * 1024 * 4; // 4 MB limit
    justMemorySetLimit(limit, archTag);
    newArch->components = JUST_MALLOC_TAGGED(sizeReq, archTag);

    // - - - make the mapping again
    for (size_t i = 0; i < COMPONENT_COUNT; ++i)
    {
      ComponentID compId                  = COMPONENT_IDS[i];
      newArch->compIdToColumnMap[compId]  = (uint8_t)i;
      size_t compSize                     = _world->componentRegistry[compId];

      bool ok = justDynamicArrayCreate(&(newArch->components[i]), 0, compSize, NULL, archTag);
      JUST_ASSERT_DEBUG_MESSAGE(ok, "[ECS] : Failed to create column array");
    }
  }
  else newArch->components = NULL;

  // - - - Get entitites ready
  bool ok = justDynamicArrayCreate(&(newArch->entityIds), 0, sizeof(EntityID), NULL, archTag);
  JUST_ASSERT_DEBUG_MESSAGE(ok, "[ECS] : Failed to create entity ID array");

  return id;
}

ArchetypeID ecsInternalFindOrCreateArchetypeByMask(uint64_t MASK)
{
  JustDynamicArray* archRegistry      = &(_world->archetypeRegistry);
  Archetype*        archs             = JUST_DARRAY_DATA(archRegistry, Archetype);
  size_t            currentArchCount  = justDynamicArraySize(archRegistry);

  // - - - search for existing archetype
  for (size_t i = 0; i < currentArchCount; ++i)
  {
    if (archs[i].mask == MASK) return i;
  }

  // - - - collect component IDs contained in a mask
  ComponentID compIDs[MAX_COMPONENT_COUNT];
  size_t count = 0;
  for (size_t i = 0; i < _world->componentIndex; ++i)
  {
    if (MASK & ((uint64_t)1 << i)) compIDs[count++] = i;
  }

  // - - - Register new archetype on demand
  return ecsRegisterArchetype(count, compIDs);
}


JUST_API void ecsChangeArchetype(EntityID ENTITY, ArchetypeID TARGET_ARCHETYPE)
{
  JUST_ASSERT_DEBUG_MESSAGE(ecsIsEntityValid(ENTITY), "[ECS] : Cannot change archetype of an invalid ENTITY");
  JUST_ASSERT_DEBUG_MESSAGE(ecsIsArchetypeValid(TARGET_ARCHETYPE), "[ECS] : Cannot change to an invalid TARGET_ARCHETYPE");
  JUST_ASSERT_DEBUG_MESSAGE(ecsGetEntityArchetype(ENTITY) != TARGET_ARCHETYPE, "[ECS] : TARGET_ARCHETYPE is the same as current archetype");

  // - - - Retrieve data on entity
  uint32_t      index  = ecsGetEntityIndex(ENTITY);
  EntityRecord* record = (EntityRecord*) justDynamicArrayAt(&(_world->entityRegistry), index);
  
  // - - - Retrieve data on archetype
  ArchetypeID srcArchID = record->archetypeID;
  Archetype*  srcArch   = ecsInternalGetArchetype(srcArchID);
  Archetype*  dstArch   = ecsInternalGetArchetype(TARGET_ARCHETYPE);
  uint32_t    srcRow    = record->rowIndex;
  uint32_t    dstRow    = (uint32_t) dstArch->entityCount;

  // 1. Allocate a new row across ALL destination columns first
  for (size_t col = 0; col < dstArch->componentCount; ++col)
  {
    justDynamicArrayEmplace(&(dstArch->components[col]));
  }

  // 2. Copy matching/common components from src columns to dst columns
  for (uint8_t componentID = 0; componentID < _world->componentIndex; ++componentID)
  {
    uint8_t srcCol = srcArch->compIdToColumnMap[componentID];
    uint8_t dstCol = dstArch->compIdToColumnMap[componentID];

    if (srcCol != INVALID_MAPPING && dstCol != INVALID_MAPPING)
    {
      void* srcSlot = justDynamicArrayAt(&(srcArch->components[srcCol]), srcRow);
      void* dstSlot = justDynamicArrayAt(&(dstArch->components[dstCol]), dstRow);
      memcpy(dstSlot, srcSlot, _world->componentRegistry[componentID]);
    }
  }

  // - - - Add Entity handle to destination archetype
  justDynamicArrayPush(&(dstArch->entityIds), &ENTITY);
  dstArch->entityCount++;

  // - - - Swap and pop dead row from source archetype
  uint32_t lastSrcRow = (uint32_t) (srcArch->entityCount - 1);
  if (srcRow != lastSrcRow)
  {
    // - - - Swap and pop all component data
    for (uint8_t col = 0; col < srcArch->componentCount; ++col)
    {
      JustDynamicArray* colArray = &(srcArch->components[col]);
      void*             deadSlot = justDynamicArrayAt(colArray, srcRow);
      void*             lastSlot = justDynamicArrayAt(colArray, lastSrcRow);

      memcpy(deadSlot, lastSlot, colArray->elementSize);
      justDynamicArrayPop(colArray, NULL);
    }

    // - - - Swap and pop entity handle
    EntityID* deadEntitySlot = (EntityID*) justDynamicArrayAt(&(srcArch->entityIds), srcRow);
    EntityID* lastEntitySlot = (EntityID*) justDynamicArrayAt(&(srcArch->entityIds), lastSrcRow);
    *deadEntitySlot = *lastEntitySlot;
    justDynamicArrayPop(&(srcArch->entityIds), NULL);
  
    // - - - Swap and pop entity registry data
    EntityID      movedEntity = *deadEntitySlot;
    uint32_t      movedIndex  = ecsGetEntityIndex(movedEntity);
    EntityRecord* movedRecord = (EntityRecord*) justDynamicArrayAt(&(_world->entityRegistry), movedIndex);
    movedRecord->rowIndex     = srcRow;
  }
  else
  {
    for (size_t col = 0; col < srcArch->componentCount; ++col)
    {
      justDynamicArrayPop(&(srcArch->components[col]), NULL);
    }
    justDynamicArrayPop(&(srcArch->entityIds), NULL);
  }
  srcArch->entityCount--;

  // - - - Point Entity record to new archetype now
  record->archetypeID = TARGET_ARCHETYPE;
  record->rowIndex    = dstRow;
}
