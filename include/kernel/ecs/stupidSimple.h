#pragma once

#include <kernel/justLibrary.h>
#include <stdint.h>

#define ECS_MAX_ENTITES     50000
#define ECS_MAX_COMPONENTS  32
#define ECS_MAX_SYSTEMS     32
#define MEMORY_TAG_ECS_COMPONENT "ECS COMPONENT"

typedef uint32_t Entity;
typedef uint32_t ComponentType;

typedef void (*SystemStartFunc) (void);
typedef void (*SystemRunFunc)   (float DELTA_TIME);
typedef void (*SystemStopFunc)  (void);

typedef struct system
{
  const char*     name;
  SystemStartFunc start;
  SystemRunFunc   run;
  SystemStopFunc  stop;
} System;

JUST_API void ecsInit(void);
JUST_API void ecsStart(void);
JUST_API void ecsRun(float DELTA_TIME);
JUST_API void ecsStop(void);
JUST_API void ecsDestroy(void);

JUST_API bool ecsRegisterSystem(System SYSTEM);

JUST_API Entity ecsCreateEntity(void);
JUST_API void   ecsDestroyEntity(Entity ENTITY);
JUST_API bool   ecsIsEntityValid(Entity ENTITY);
JUST_API size_t ecsGetMaxEntities(void);

JUST_API ComponentType  ecsRegisterComponent  (size_t SIZE);
JUST_API void*          ecsAddComponent       (Entity ENTITY, ComponentType TYPE, const void* DATA);
JUST_API void           ecsRemoveComponent    (Entity ENTITY, ComponentType TYPE);
JUST_API bool           ecsHasComponent       (Entity ENTITY, ComponentType TYPE);
JUST_API void*          ecsGetComponent       (Entity ENTITY, ComponentType TYPE);
