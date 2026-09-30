#include <cstdint>
#include <kernel/ecs/world.h>
#include <stddef.h>
#include <limits.h>
#include <stdint.h>


typedef struct entityRecord
{
  size_t    archetypeIndex;
  EntityID  id;
} EntityRecord;

typedef size_t ComponentInfo;

typedef struct ecsWorld
{
  JustDynamicArray  entityRegistry;
  JustDynamicArray  archetypeRegistry;
  size_t            componentIndex;
  ComponentInfo     componentRegistry[];
} ECSWorld;

static ECSWorld*    world     = NULL;
static const char*  worldTag  = "ECS_WORLD";

JUST_API void ecsInit(void)
{
  JUST_LOG_DEBUG("Trying to initialize ECS");

  JUST_ASSERT_DEBUG_MESSAGE(world == NULL, "[ECS WORLD] : Trying to double initialize ECS");

  size_t noLimit = 0;
  justMemorySetLimit(noLimit, worldTag);

  size_t memNeeded = sizeof(ECSWorld) + (MAX_COMPONENT_COUNT * sizeof(ComponentInfo));
  world = JUST_MALLOC_TAGGED(memNeeded, worldTag);
  JUST_ASSERT_DEBUG_MESSAGE(world != NULL, "[ECS WORLD] : Failed to allocate enough memory ");

  bool ok = JUST_DARRAY_INIT_TAGGED(&(world->entityRegistry), 0, EntityRecord, worldTag);
  JUST_ASSERT_DEBUG_MESSAGE(ok, "[ECS WORLD] : Could not initialize entity registry");
  JUST_LOG_TRACE("[ECS WORLD] : Initialized entity registry");

  ok = JUST_DARRAY_INIT_TAGGED(&(world->archetypeRegistry), 0, Archetype, worldTag);
  JUST_ASSERT_DEBUG_MESSAGE(ok, "[ECS WORLD] : Could not initialize archetype registry");
  JUST_LOG_TRACE("[ECS WORLD] : Initialized archetype registry");

  for (uint8_t i = 0; i < MAX_COMPONENT_COUNT; ++i) world->componentRegistry[i] = SIZE_MAX;
  JUST_LOG_TRACE("[ECS WORLD] : Initialized component registry");

  JUST_LOG_INFO("[ECS WORLD] : Initialized");
}

JUST_API void ecsShutdown(void)
{
  JUST_LOG_DEBUG("[ECS WORLD] : Shutting down");

  JUST_ASSERT_DEBUG_MESSAGE(world != NULL, "[ECS WORLD] : Trying to double shutdown or shutting down an unitialized ECS");

  JUST_FREE(&(world->entityRegistry));
  JUST_LOG_TRACE("[ECS WORLD] : Entity Registry freed");

  JUST_FREE(&(world->archetypeRegistry));
  JUST_LOG_TRACE("[ECS WORLD] : Component Registry freed");

  JUST_FREE(world);
  world = NULL;

  JUST_LOG_INFO("[ECS WORLD] : Shut down");
}

JUST_API size_t ecsRegisterComponent(size_t SIZE)
{
  JUST_ASSERT_DEBUG_MESSAGE(SIZE > 0, "[ECS WORLD] : Cant register component of SIZE 0");
 
  
}
