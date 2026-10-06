#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <kernel/ecs/world.h>

typedef struct { float x, y, z; } Position;
typedef struct { float dx, dy, dz; } Velocity;

int32_t main(void)
{
  ecsInit();
  ComponentID positionComponent = ECS_REGISTER_COMPONENT(Position);
  ComponentID velocityComponent = ECS_REGISTER_COMPONENT(Velocity);
  JUST_LOG_TRACE("Position Component Registerd : %zu", positionComponent);
  JUST_LOG_TRACE("Velocity Component Registerd : %zu", velocityComponent);
  justMemoryLogUsage(true);

  ComponentID stationaryComponents[] = { positionComponent };
  ArchetypeID stationaryArchetype = ecsRegisterArchetype(1, stationaryComponents);
  JUST_LOG_TRACE("Stationary Archetype Registerd : %zu", stationaryArchetype);
  justMemoryLogUsage(true);

  EntityID box = ecsCreateEntity(stationaryArchetype);
  JUST_LOG_TRACE("Created a box entity : %zu", box);
  justMemoryLogUsage(true);
}

