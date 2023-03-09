/*
   Copyright 2021 MacKenzie Strand

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#pragma once

#include "utilities/Common.h"

typedef uint32_t ctSceneEntity;
typedef uint32_t ctSceneComponentType;
typedef struct ctSceneComponentQueryT* ctSceneComponentQuery;
typedef struct ctSceneComponentFetcherT* ctSceneComponentFetcher;
typedef struct ctSceneContextT* ctSceneContext;

typedef void (*ctSceneDestructorFunc)(void* pComponentData, void* pUserData);
typedef bool (*ctSceneCustomFilterFunc)(const void* pComponentData, const void* pUserData);

// clang-format off
enum ctResults ctSceneContextCreate(ctSceneContext* pContext);
enum ctResults ctSceneContextDestroy(ctSceneContext context);

struct ctSceneComponentReflectData {
   const char* name;
   const char* datatype;
   size_t offset;
   size_t arrayLength;
};

#define CT_SCENE_COM_REFLECT(_COMPONENT, _TYPE, _MEMBER) { ##_DATA, ##_TYPE, offsetof(_COMPONENT, _MEMBER), 0}
#define CT_SCENE_COM_REFLECT_ARRAY(_COMPONENT, _TYPE, _MEMBER, _COUNT) { ##_DATA, ##_TYPE, offsetof(_COMPONENT, _MEMBER), _COUNT}
#define CT_SCENE_COM_REFLECT_END() { NULL, NULL, 0, 0}

struct ctSceneComponentDesc {
   inline ctSceneComponentDesc(){}
   inline ctSceneComponentDesc(const char* name) {
      this->componentName = name;
   }
   inline ctSceneComponentDesc(const char* name, size_t size) {
      this->componentName = name;
      this->componentSize = size;
   }
   inline ctSceneComponentDesc(const char* name, size_t size, void* pTemplate) {
      this->componentName = name;
      this->componentSize = size;
      this->pDefaultTemplate = pTemplate;
   }
   const char* componentName = NULL;
   size_t componentSize = 0;
   void* pDefaultTemplate = NULL;
   ctSceneComponentReflectData* pReflection = NULL;
   ctSceneDestructorFunc fpDestructor = NULL;
   void* pUserData = NULL;
};

enum ctResults ctSceneComponentTypeRegister(ctSceneContext context, ctSceneComponentDesc& desc);
enum ctResults ctSceneComponentGetReflection(ctSceneContext context, const char* name, ctSceneComponentReflectData** ppReflection);

enum ctResults ctSceneEntityCreate(ctSceneContext context, ctSceneEntity& entity);
enum ctResults ctSceneEntityRelease(ctSceneContext context, ctSceneEntity entity);

enum ctResults ctSceneComponentCreate(ctSceneContext context, ctSceneEntity entity, const char* name, const void* pContents = NULL);
enum ctResults ctSceneComponentRelease(ctSceneContext context, ctSceneEntity entity, const char* name);

enum ctSceneSystemInputFlags {
   CT_SCENE_INPUT_READ,
   CT_SCENE_INPUT_WRITE,
   CT_SCENE_INPUT_READ_WRITE,
   CT_SCENE_INPUT_OPTIONAL,
   CT_SCENE_INPUT_FETCH,
   CT_SCENE_INPUT_TAG,
   CT_SCENE_INPUT_EXCLUDE
};

enum ctSceneSystemQueryStrategy {
   CT_SCENE_QUERY_CACHE,
   CT_SCENE_QUERY_JIT
};

class ctSceneSystemDefineCtx {
public:
   void SetQueryStrategy(ctSceneSystemQueryStrategy strategy);
   void DeclareQueryInput(const char* componentName, int32_t flags, ctSceneCustomFilterFunc* fpFilter = NULL, const void* pUserData = NULL);
   void AcquireLock(const char* lockName);
};

struct ctSceneSystemExecuteCtx{
   void* ptrs;
   int32_t ptrcount;
   int32_t loopCount;
   void* jitData;
};

#define ctSceneCacheSystemExecSetup(_CTX)  void* ___CTX_PT = &_CTX; int32_t ___COM_CT = _CTX.ptrcount; void* ___COM_PT = _CTX.ptrs; int32_t ___LOOP_CT = _CTX.loopCount;
#define ctSceneCacheSystemLoop() for(int32_t ___LOOP_ID = 0; ___LOOP_ID < ___LOOP_CT; ___LOOP_ID++)
#define ctSceneCacheGetComponentPtr(_TYPE, _IDX) (_TYPE*)(___COM_PT + (___LOOP_ID * ___COM_CT) + _IDX)
#define ctSceneCacheGetComponent(_TYPE, _IDX) *ctSceneGetComponentPtr(_TYPE, _IDX)

void* _ctSceneJITLookupImpl(int32_t i, ctSceneSystemExecuteCtx* pCtx);
bool _ctSceneJITLookupIterate(ctSceneSystemExecuteCtx* pCtx);
#define ctSceneJITSystemExecSetup(_CTX)  void* ___CTX_PT = &_CTX;
#define ctSceneJITSystemLoop() while(_ctSceneJITLookupIterate(___CTX_PT))
#define ctSceneJITGetComponentPtr(_TYPE, _IDX) (_TYPE*)_ctSceneJITLookupImpl(_IDX, ___CTX_PT);
#define ctSceneJITGetComponent(_TYPE, _IDX) *ctSceneGetComponentPtr(_TYPE, _IDX)

void* _ctSceneFetchPtrImpl(size_t hash, ctSceneEntity entity, ctSceneSystemExecuteCtx* pCtx);
#define ctSceneFetchComponentPtr(_TYPE, _ENTITY, _NAME) (_TYPE*)_ctSceneFetchPtrImpl(CT_COMPILE_HORNER_HASH(_NAME), _ENTITY, ___CTX_PT);

class ctSceneSystemBase {
public:
   virtual ctResults OnDefinition(ctSceneSystemDefineCtx& ctx) = 0;
   virtual ctResults OnExecute(ctSceneSystemExecuteCtx& ctx) = 0;
};

enum ctResults ctSceneSystemRegister(ctSceneContext context, const char* name, ctSceneSystemBase* pSystem);

enum ctResults ctSceneScheduleSystem(ctSceneContext context, const char* name);
enum ctResults ctSceneExecuteScheduled(ctSceneContext context, void* taskHandler);
enum ctResults ctSceneResetScheduled(ctSceneContext context);
// clang-format on