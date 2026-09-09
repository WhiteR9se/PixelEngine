#include "kernel/utils/assert.h"
#include <kernel/defines.h>
#include <kernel/ecs/stupidSimple.h>
#include <kernel/utils/logger.h>
#include <kernel/memory/tracker.h>
#include <stddef.h>
#include <stdint.h>

typedef struct ecsWorld
{
  bool entityActive[ECS_MAX_ENTITES];

  size_t  componentSizes[ECS_MAX_COMPONENTS];
  void*   componentData[ECS_MAX_COMPONENTS];
  bool    hasComponent[ECS_MAX_ENTITES][ECS_MAX_COMPONENTS];
  size_t  registeredComponentCount;

  System systems[ECS_MAX_SYSTEMS];
  size_t registeredSystemCount;
} ECSWorld;

static ECSWorld world = {0};

ENGINE_API void ecsInit(void) 
{ memset(&world, 0, sizeof(ECSWorld)); }

ENGINE_API void ecsStart(void)
{
  for (size_t i = 0; i < world.registeredSystemCount; ++i)
  {
    if (world.systems[i].start)
    {
      LOG_INFO("[ECS] : Starting system %s", world.systems[i].name);
      world.systems[i].start();
    }
    else
    {
      LOG_WARNING("[ECS] : System %s has no start function", world.systems[i].name);
    }
  }
}

ENGINE_API void ecsRun(float DELTA_TIME)
{
  for (size_t i = 0; i < world.registeredSystemCount; ++i)
  {
    if (world.systems[i].run) world.systems[i].run(DELTA_TIME);
  }
}

ENGINE_API void ecsStop(void)
{
  for (size_t i = 0; i < world.registeredSystemCount; ++i)
  {
    if (world.systems[i].stop)
    {
      LOG_INFO("[ECS] : Stopping system %s", world.systems[i].name);
      world.systems[i].stop();
    }
    else
    {
      LOG_WARNING("[ECS] : System %s has no stop function", world.systems[i].name);
    }
  }
}

ENGINE_API void ecsDestroy(void)
{
  ecsStop();
  for (size_t i = 0; i < world.registeredComponentCount; ++i)
  {
    ENGINE_FREE(world.componentData[i]);
    world.componentData[i] = NULL;
  }
  memset(&world, 0, sizeof(ECSWorld));
}

ENGINE_API bool ecsRegisterSystem(System SYSTEM)
{
  if (world.registeredSystemCount >= ECS_MAX_SYSTEMS)
  {
    LOG_WARNING("[ECS] : Already registered max systems, no space left for %s", SYSTEM.name);
    return false;
  }

  world.systems[world.registeredSystemCount] = SYSTEM;
  world.registeredSystemCount++;
  return true;
}

ENGINE_API Entity ecsCreateEntity(void)
{
  for (Entity i = 0; i < ECS_MAX_ENTITES; ++i)
  {
    if (!world.entityActive[i])
    {
      world.entityActive[i] = true;
      for (size_t comp = 0; comp < ECS_MAX_COMPONENTS; comp++)
      {
        world.hasComponent[i][comp] = false;
      }
      return i;
    }
  }
  return (Entity) -1;
}

ENGINE_API void ecsDestroyEntity(Entity ENTITY)
{
  if (ENTITY >= ECS_MAX_ENTITES || !world.entityActive[ENTITY]) return;

  world.entityActive[ENTITY] = false;
  for (size_t comp = 0; comp < ECS_MAX_COMPONENTS; ++comp)
  {
    world.hasComponent[ENTITY][comp] = false;
  }
}

ENGINE_API bool ecsIsEntityValid(Entity ENTITY)
{
  if (ENTITY >= ECS_MAX_ENTITES) return false;
  return world.entityActive[ENTITY];
}

ENGINE_API size_t ecsGetMaxEntities(void)
{ return ECS_MAX_ENTITES; }

ENGINE_API ComponentType ecsRegisterComponent(size_t SIZE)
{
  if (world.registeredComponentCount >= ECS_MAX_COMPONENTS)
  {
    LOG_WARNING("[ECS] : Cannot register new component, no memory left");
    return (ComponentType) -1;
  }

  ComponentType type          = (ComponentType) world.registeredComponentCount;
  world.componentSizes[type]  = SIZE;
  world.componentData[type]   = ENGINE_CALLOC(ECS_MAX_ENTITES, SIZE, MEMORY_TAG_ECS_COMPONENT);
  world.registeredComponentCount++;

  return type;
}

ENGINE_API void* ecsAddComponent(Entity ENTITY, ComponentType TYPE, const void* DATA)
{
  ASSERT_DEBUG_MESSAGE(DATA != NULL, "[ECS] : Cannot add component with NULL DATA");

  if (!ecsIsEntityValid(ENTITY))
  {
    LOG_ERROR("[ECS] : Trying to add component to invalid entity : %u", ENTITY);
    return NULL;
  }
  if (TYPE >= world.registeredComponentCount)
  {
    LOG_ERROR("[ECS] : Trying to add invalid component to entity : %u", TYPE);
    return NULL;
  }

  world.hasComponent[ENTITY][TYPE] = true;
  void* slot = (uint8_t*) world.componentData[TYPE] + (ENTITY * world.componentSizes[TYPE]);

  memcpy(slot, DATA, world.componentSizes[TYPE]);
  return slot;
}

ENGINE_API void ecsRemoveComponent(Entity ENTITY, ComponentType TYPE)
{
  if (!ecsIsEntityValid(ENTITY))
  {
    LOG_ERROR("[ECS] : Trying to remove component from invalid entity : %u", ENTITY);
    return;
  }
  if (TYPE >= world.registeredComponentCount)
  {
    LOG_ERROR("[ECS] : Trying to remove invalid component from entity : %u", TYPE);
    return;
  }

  world.hasComponent[ENTITY][TYPE] = false;
}

ENGINE_API bool ecsHasComponent(Entity ENTITY, ComponentType TYPE)
{

  if (!ecsIsEntityValid(ENTITY))
  {
    LOG_ERROR("[ECS] : Trying to check component from invalid entity : %u", ENTITY);
    return false;
  }
  if (TYPE >= world.registeredComponentCount)
  {
    LOG_ERROR("[ECS] : Trying to check invalid component from entity : %u", TYPE);
    return false;
  }

  return world.hasComponent[ENTITY][TYPE];
}

ENGINE_API void* ecsGetComponent(Entity ENTITY, ComponentType TYPE)
{
  if (!ecsHasComponent(ENTITY, TYPE))
  {
    return NULL;
  }

  return (uint8_t*) world.componentData[TYPE] + (ENTITY * world.componentSizes[TYPE]);
}
