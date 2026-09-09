#include <kernel/memory/tracker.h>
#include <kernel/ecs/stupidSimple.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>

typedef struct position
{ float x, y, z; } Position;

typedef struct velocity
{ float vx, vy, vz; } Velocity;

static ComponentType COMP_POSITION;
static ComponentType COMP_VELOCITY;

static void lightMovementSystem(float DELTA_TIME)
{
  size_t  max         = ecsGetMaxEntities();
  float   accumulator = 0.0f;

  for (Entity e = 0; e < max; e++)
  {
    if (!ecsIsEntityValid(e)) continue;

    if (ecsHasComponent(e, COMP_POSITION) &&
        ecsHasComponent(e, COMP_VELOCITY))
    {
      Position* p = (Position*) ecsGetComponent(e, COMP_POSITION);
      Velocity* v = (Velocity*) ecsGetComponent(e, COMP_VELOCITY);

      p->x += v->vx * DELTA_TIME;
      p->y += v->vy * DELTA_TIME;
      p->z += v->vz * DELTA_TIME;

      accumulator += p->x;
    }
  }

  accumulator = accumulator + 1;
}

static void heavyMovementSystem(float DELTA_TIME)
{
  size_t  max         = ecsGetMaxEntities();
  float   accumulator = 0.0f;

  for (Entity e = 0; e < max; ++e)
  {
    if (!ecsIsEntityValid(e)) continue;

    if (ecsHasComponent(e, COMP_POSITION) &&
        ecsHasComponent(e, COMP_VELOCITY))
    {
      Position* p = (Position*) ecsGetComponent(e, COMP_POSITION);
      Velocity* v = (Velocity*) ecsGetComponent(e, COMP_VELOCITY);

      float dist = sqrtf(p->x * p->x + p->y * p->y * p->z * p->z) + 0.0001f;
      float drag = sinf(dist) * 0.01f;

      v->vx -= (p->x / dist) * drag * DELTA_TIME;
      v->vy -= (p->y / dist) * drag * DELTA_TIME;
      v->vz -= (p->z / dist) * drag * DELTA_TIME;

      accumulator += v->vx + dist;
    }
    accumulator = accumulator + 1;
  }
}

int32_t main(void)
{
  memorySetLimit(1024 * 1024 * 2, MEMORY_TAG_ECS_COMPONENT);
  ecsInit();

  COMP_POSITION = ecsRegisterComponent(sizeof(Position));
  COMP_VELOCITY = ecsRegisterComponent(sizeof(Velocity));

  ecsRegisterSystem((System)
     {
      .name = "LIGHT MOVEMENT",
      .run  = lightMovementSystem,
    });
  ecsRegisterSystem((System)
     {
      .name = "HEAVY MOVEMENT",
      .run  = heavyMovementSystem,
    });

  size_t maxEntities = ecsGetMaxEntities();
  for (size_t i = 0; i < maxEntities; ++i)
  {
    Entity e = ecsCreateEntity();

    Position pos = { (float)i, (float)(i * 2), (float)(i * 3) };
    ecsAddComponent(e, COMP_POSITION, &pos);

    if (i % 2 == 0)
    {
      Velocity vel = { 1.0f, 0.5f, 0.25f };
      ecsAddComponent(e, COMP_VELOCITY, &vel);
    }
  }

  ecsStart();

  const int32_t BENCHMARK_FRAMES  = 5000;
  const float   DT                = 0.0166f;

  for (size_t frame = 0; frame < BENCHMARK_FRAMES; ++frame)
  { ecsRun(DT); }

  memoryLogUsageStr();

  ecsDestroy();
}
