#pragma once

#include <kernel/defines.h>
#include <stdint.h>

#define ECS_MAX_ENTITES     50000
#define ECS_MAX_COMPONENTS  32
#define ECS_MAX_SYSTEMS     32

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

ENGINE_API void ecsInit(void);
ENGINE_API void ecsStart(void);
ENGINE_API void ecsRun(float DELTA_TIME);
ENGINE_API void ecsStop(void);
ENGINE_API void ecsDestroy(void);

ENGINE_API bool ecsRegisterSystem(System SYSTEM);

ENGINE_API Entity ecsCreateEntity(void);
ENGINE_API void   ecsDestroyEntity(Entity ENTITY);
ENGINE_API bool   ecsIsEntityValid(Entity ENTITY);
ENGINE_API size_t ecsGetMaxEntities(void);

ENGINE_API ComponentType  ecsRegisterComponent  (size_t SIZE);
ENGINE_API void*          ecsAddComponent       (Entity ENTITY, ComponentType TYPE, const void* DATA);
ENGINE_API void           ecsRemoveComponent    (Entity ENTITY, ComponentType TYPE);
ENGINE_API bool           ecsHasComponent       (Entity ENTITY, ComponentType TYPE);
ENGINE_API void*          ecsGetComponent       (Entity ENTITY, ComponentType TYPE);
