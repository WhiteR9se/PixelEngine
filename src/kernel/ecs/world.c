#include <kernel/ecs/world.h>
#include <stddef.h>
#include <limits.h>
#include <stdint.h>

/// @brief : Only thing to note about a component is its size
typedef size_t ComponentInfo;

/// @brief : defines an archetype
typedef struct archetype
{
  ArchetypeID       mask;                                   ///< The bitmask of an archetype
  size_t            entityCount;                            ///< How many entities of this archectype 
  size_t            componentCount;                         ///< How many components in this archetype
  JustDynamicArray* components;                             ///< An array of dynamic arrays of components, vector<vector<Component>>
  uint8_t           compIdToColumnMap[MAX_COMPONENT_COUNT]; ///< Maps component ids to column index
} Archetype;

/// @brief : global state of the ecs system
typedef struct ecsWorld
{
  JustDynamicArray  archetypeRegistry;
  size_t            componentIndex;
  ComponentInfo     componentRegistry[];
} ECSWorld;

static ECSWorld*    world     = NULL;
static const char*  worldTag  = "ECS_WORLD";

#define ENSURE_AFTER_INIT JUST_ASSERT_DEBUG(world != NULL);

JUST_API void ecsInit(void)
{
  JUST_LOG_DEBUG("Trying to initialize ECS");

  JUST_ASSERT_DEBUG_MESSAGE(world == NULL, "[ECS WORLD] : Trying to double initialize ECS");

  size_t noLimit = 0;
  justMemorySetLimit(noLimit, worldTag);

  size_t memNeeded = sizeof(ECSWorld) + (MAX_COMPONENT_COUNT * sizeof(ComponentInfo));
  world = JUST_MALLOC_TAGGED(memNeeded, worldTag);
  JUST_ASSERT_DEBUG_MESSAGE(world != NULL, "[ECS WORLD] : Failed to allocate enough memory ");

  bool ok = JUST_DARRAY_INIT_TAGGED(&(world->archetypeRegistry), 0, Archetype, worldTag);
  JUST_ASSERT_DEBUG_MESSAGE(ok, "[ECS WORLD] : Could not initialize archetype registry");
  JUST_LOG_TRACE("[ECS WORLD] : Initialized archetype registry");

  for (uint8_t i = 0; i < MAX_COMPONENT_COUNT; ++i) world->componentRegistry[i] = SIZE_MAX;
  world->componentIndex = 0;
  JUST_LOG_TRACE("[ECS WORLD] : Initialized component registry");

  JUST_LOG_INFO("[ECS WORLD] : Initialized");
}

JUST_API void ecsShutdown(void)
{
  JUST_LOG_DEBUG("[ECS WORLD] : Shutting down");

  JUST_ASSERT_DEBUG_MESSAGE(world != NULL, "[ECS WORLD] : Trying to double shutdown or shutting down an unitialized ECS");

  // - - - Clear all the archetypes
  JustDynamicArray* archRegistry  = &(world->archetypeRegistry);
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
  JUST_LOG_TRACE("[ECS WORLD] : Archetype Registry destroyed");

  // - - - Clear out the component registy
  world->componentIndex = 0;
  JUST_LOG_TRACE("[ECS WORLD] : Component Registry destroyed");

  // - - - Destroy the world
  JUST_FREE(world);
  world = NULL;

  JUST_LOG_INFO("[ECS WORLD] : Shut down");
}

JUST_API size_t ecsRegisterComponent(size_t SIZE)
{
  JUST_ASSERT_DEBUG_MESSAGE(SIZE > 0, "[ECS WORLD] : Cant register component of SIZE 0");

  ENSURE_AFTER_INIT

  size_t index                    = world->componentIndex;
  if (index == MAX_COMPONENT_COUNT)
  {
    JUST_LOG_FATAL("[ECS WORLD] : Only %d components can be registered", MAX_COMPONENT_COUNT);
    return INVALID_COMPONENT;
  }

  world->componentRegistry[index] = SIZE;
  world->componentIndex++;

  JUST_LOG_INFO("[ECS WORLD] : Registered Component %d with size %zu", index, SIZE);
  return index;
}

JUST_API size_t ecsRegisterArchetype(size_t COMPONENT_COUNT, ComponentID COMPONENT_IDS[])
{
  ENSURE_AFTER_INIT

  // - - - Make the mask
  ArchetypeID mask = 0;
  for (uint8_t i = 0; i < COMPONENT_COUNT; ++i)
  {
    JUST_ASSERT_DEBUG_MESSAGE(COMPONENT_IDS[i] < world->componentIndex, "[ECS WORLD] : Trying to register an archetype with invalid COMPONENT_IDS");
    mask |= ((uint64_t)1 << COMPONENT_IDS[i]);
  }

  // - - - Check duplicates
  Archetype* archs = JUST_DARRAY_DATA(&(world->archetypeRegistry), Archetype);
  for (size_t i = 0; i < justDynamicArraySize(&(world->archetypeRegistry)); ++i)
  {
    JUST_ASSERT_DEBUG_MESSAGE(archs[i].mask != mask, "[ECS WORLD] : Trying to insert duplicate archetype");
  }

  // - - - Create the new archetype
  Archetype*  newArchetype      = JUST_DARRAY_EMPLACE(&(world->archetypeRegistry), Archetype);
  ArchetypeID id                = justDynamicArraySize(&(world->archetypeRegistry)) - 1;
  newArchetype->mask            = mask;
  newArchetype->componentCount  = COMPONENT_COUNT;
  newArchetype->entityCount     = 0;
  newArchetype->components      = NULL;

  // - - - Invalidate all mappings
  #define INVALID_MAPPING 0xFF
  memset(newArchetype->compIdToColumnMap, INVALID_MAPPING, sizeof(newArchetype->compIdToColumnMap));

  // - - - Make enough dynamic arrays
  char tag[32];
  snprintf(tag, sizeof(tag), "Archetype_%zu", id);
  size_t sizeReq            = sizeof(JustDynamicArray) * COMPONENT_COUNT;
  newArchetype->components  = JUST_MALLOC_TAGGED(sizeReq, tag);
  JUST_ASSERT_DEBUG_MESSAGE(newArchetype->components != NULL, "[ECS WORLD] : Failed to allocate memory for archetype");

  // - - - Initialize the relevant dynamic arrays and map them
  for (uint8_t i = 0; i < COMPONENT_COUNT; ++i)
  {
    ComponentID compId                      = COMPONENT_IDS[i];
    newArchetype->compIdToColumnMap[compId] = i;
    size_t            componentSize         = world->componentRegistry[compId];
    JustDynamicArray* componentMemory       = &(newArchetype->components[i]);

    bool ok = justDynamicArrayCreate(componentMemory, 0, componentSize, NULL, tag);
    JUST_ASSERT_DEBUG_MESSAGE(ok, "[ECS WORLD] : Failed to initialize space for component");

    (void) ok;
  }
  (void) tag;

  return id;
}