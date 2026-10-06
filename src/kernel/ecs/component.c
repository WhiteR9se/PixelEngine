/**
 * @file : component.c
 * @brief : Handles component related tasks
*/

#include <kernel/ecs/_internal.h>
#include <stdint.h>


JUST_API ComponentID ecsRegisterComponent(size_t SIZE)
{
  JUST_LOG_DEBUG("[ECS] : Trying to register a component with size %zu", SIZE);

  ENSURE_AFTER_INIT
  JUST_ASSERT_DEBUG_MESSAGE(SIZE > 0, "[ECS] : SIZE must be greater than 0 to register a component");

  size_t index = _world->componentIndex;
  JUST_ASSERT_DEBUG_MESSAGE(index < MAX_COMPONENT_COUNT, "[ECS] : Component limit reached");

  _world->componentRegistry[index] = SIZE;
  _world->componentIndex++;

  JUST_LOG_DEBUG("[ECS] : Component Registered : %zu with size : %zu", index, SIZE);
  return index;
}

JUST_API void* ecsGetComponent(EntityID ENTITY, ComponentID COMPONENT)
{
  ENSURE_AFTER_INIT
  JUST_ASSERT_DEBUG_MESSAGE(ecsIsComponentValid(COMPONENT), "[ECS] : Invalid COMPONENT");
  JUST_ASSERT_DEBUG_MESSAGE(ecsIsEntityValid(ENTITY), "[ECS] : Invlaid ENTITY");

  uint32_t      index   = ecsGetEntityIndex(ENTITY);
  EntityRecord* record  = (EntityRecord*) justDynamicArrayAt(&(_world->entityRegistry), index);
  Archetype*    arch    = ecsInternalGetArchetype(record->archetypeID);
  uint8_t       col     = arch->compIdToColumnMap[COMPONENT];

  if (col == INVALID_MAPPING) return NULL;
  else                        return justDynamicArrayAt(&(arch->components[col]), record->rowIndex);
}

JUST_API void ecsSetComponent(EntityID ENTITY, ComponentID COMPONENT, const void* DATA)
{
  JUST_ASSERT_DEBUG_MESSAGE(DATA != NULL, "[ECS] : Cannot set component with NULL DATA");

  void* dst = ecsGetComponent(ENTITY, COMPONENT);
  JUST_ASSERT_DEBUG_MESSAGE(dst != NULL, "[ECS] : Cannot set because ENTITY does not have COMPONENT");

  size_t componentSize = _world->componentRegistry[COMPONENT];
  memcpy(dst, DATA, componentSize);
}
