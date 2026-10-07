#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <kernel/ecs/world.h>

// --- Component Definitions ---
typedef struct { float x, y, z; } Position;
typedef struct { float vx, vy, vz; } Velocity;
typedef struct { float hp, maxHp; } Health;
typedef struct { float radius; } Collider;

#define ENTITY_COUNT 1000000
#define FRAME_COUNT  300


void moveCall(ECSSystemView* view) 
{
  // Inline lambda/callback for movement update
  Position* pos = (Position*)view->components[0]; // assuming posID is 0
  Velocity* vel = (Velocity*)view->components[1]; // assuming velID is 1

  // High-throughput vector loop
  for (size_t i = 0; i < view->entityCount; ++i)
  {
    pos[i].x += vel[i].vx * 0.016f;
    pos[i].y += vel[i].vy * 0.016f;
    pos[i].z += vel[i].vz * 0.016f;
  }
}

int main(int argc, char** argv)
{
  printf("[BENCHMARK] Initializing ECS World...\n");
  ecsInit();

  // 1. Register Components
  ComponentID posID  = ECS_REGISTER_COMPONENT(Position);
  ComponentID velID  = ECS_REGISTER_COMPONENT(Velocity);
  ComponentID hpID   = ECS_REGISTER_COMPONENT(Health);
  ComponentID colID  = ECS_REGISTER_COMPONENT(Collider);

  // 2. Register Archetype Variants
  ComponentID moveComps[]   = { posID, velID };
  ComponentID fullComps[]   = { posID, velID, hpID, colID };
  ComponentID staticComps[] = { posID, hpID };

  ArchetypeID moveArch   = ecsRegisterArchetype(2, moveComps);
  ArchetypeID fullArch   = ecsRegisterArchetype(4, fullComps);
  ArchetypeID staticArch = ecsRegisterArchetype(2, staticComps);

  justMemorySetLimit(0, "ECS");
  justMemorySetLimit(0, "ECS_Arch_1");
  justMemorySetLimit(0, "ECS_Arch_2");
  justMemorySetLimit(0, "ECS_Arch_3");

  // 3. Register System
  ECSSystem moveSystem = ecsRegisterSystem(2, moveComps, moveCall);

  printf("[BENCHMARK] Spawning %d entities across archetypes...\n", ENTITY_COUNT);

  // Allocate handles array to hold spawned entities
  EntityID* entities = (EntityID*)malloc(sizeof(EntityID) * ENTITY_COUNT);

  for (int i = 0; i < ENTITY_COUNT; ++i)
  {
    ArchetypeID arch = (i % 3 == 0) ? moveArch : ((i % 3 == 1) ? fullArch : staticArch);
    entities[i] = ecsCreateEntity(arch);

    // Initialize initial values
    if (arch == moveArch || arch == fullArch)
    {
      ECS_SET_COMPONENT(entities[i], posID, ((Position){ 0.0f, 0.0f, 0.0f }));
      ECS_SET_COMPONENT(entities[i], velID, ((Velocity){ 1.0f, 2.0f, 0.5f }));
    }
  }

  printf("[BENCHMARK] Running main loop (%d frames)...\n", FRAME_COUNT);

  // 4. Main Simulation Loop (This is where `perf` will spend 99% of its time)
  for (int frame = 0; frame < FRAME_COUNT; ++frame)
  {
    // A. Iterate Query/System across matching entities
    ecsRunSystem(&moveSystem);

    // B. Stress-test archetype migration overhead: Move 1000 entities every frame
    for (int k = 0; k < 1000; ++k)
    {
      int targetIdx = (frame * 1000 + k) % ENTITY_COUNT;
      ArchetypeID currentArch = ecsGetEntityArchetype(entities[targetIdx]);
      
      if (currentArch == moveArch)
      {
        ecsChangeArchetype(entities[targetIdx], fullArch);
        ECS_SET_COMPONENT(entities[targetIdx], hpID, ((Health){ 100.0f, 100.0f }));
      }
      else if (currentArch == fullArch)
      {
        ecsChangeArchetype(entities[targetIdx], moveArch);
      }
    }
  }

  printf("[BENCHMARK] Shutting down...\n");
  free(entities);
  ecsDestroySystem(&moveSystem);
  ecsShutdown();

  return 0;
}
