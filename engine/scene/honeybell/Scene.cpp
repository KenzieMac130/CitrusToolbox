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

#include "Scene.hpp"
#include "core/EngineCore.hpp"

#include "middleware/PhysXIntegration.hpp"
#include "scripting/api/HoneybellScript.hpp"

#include "asset/AssetManager.hpp"
#include "asset/types/WAD.hpp"

#if CITRUS_PHYSX
#define PHYSX_SCRATCH_MEMORY_AMOUNT 0
#endif

ctHoneybell::Scene::Scene() {
   toyHandleManager = ctHandleManager();
}

ctHoneybell::Scene::~Scene() {
}

ctHandle ctHoneybell::Scene::_RegisterToy(ToyBase* toy) {
   ZoneScoped;
   /* Todo: Investigate hang up */
   ctHandle toyHandle = toyHandleManager.GetNewHandle();
   CT_PANIC_UNTRUE(toys.Insert(toyHandle, toy),
                   "Internal scene error: Failed to add toy");
   BeginContext ctx = BeginContext();
   toy->OnBegin(ctx);
   if (ctx.canTickSerial) { toys_TickSerial.Append(toy); }
   if (ctx.canTickParallel) { toys_TickParallel.Append(toy); }
   if (ctx.canFrameUpdate) { toys_FrameUpdate.Append(toy); }
   return toyHandle;
}

void ctHoneybell::Scene::_UnregisterToy(ctHandle hndl) {
   ZoneScoped;
   ToyBase** toy = toys.FindPtr(hndl);
   if (toy) {
      toys_TickSerial.Remove(*toy);
      toys_TickParallel.Remove(*toy);
      toys_FrameUpdate.Remove(*toy);
      toyHandleManager.FreeHandle(hndl);
      toys.Remove(hndl);
   }
}

void doParallelTickJob(void* pData) {
   ZoneScoped;
   struct jobData {
      ctHoneybell::TickContext tickCtx;
      ctHoneybell::ToyBase* pToy;
   };
   jobData* job = (jobData*)pData;
   job->pToy->OnTickParallel(job->tickCtx);
}

ctResults ctHoneybell::Scene::Startup() {
   ZoneScoped;

#if CITRUS_PHYSX
   globalGravity = ctVec3(0.0f, -9.81f, 0.0f);
   PxSceneDesc sceneDesc = PxSceneDesc(Engine->PhysXIntegration->toleranceScale);
   sceneDesc.flags |= PxSceneFlag::eENABLE_CCD;
   sceneDesc.cpuDispatcher = Engine->PhysXIntegration->pCpuDispatcher;
   sceneDesc.filterShader = PxDefaultSimulationFilterShader;
   sceneDesc.gravity = ctVec3ToPx(globalGravity);
   physMemory = ctAlignedMalloc(PHYSX_SCRATCH_MEMORY_AMOUNT, 16);
   pPxScene = Engine->PhysXIntegration->pPhysics->createScene(sceneDesc);
   componentFactory_PhysXActor.pPxScene = pPxScene;
   componentFactory_PhysXController.pPxScene = pPxScene;
#endif

   /* Startup component factories */
   componentFactory_Camera.ModuleStartup(Engine);
   componentFactory_DebugShape.ModuleStartup(Engine);
   componentFactory_PhysXActor.ModuleStartup(Engine);
   componentFactory_PhysXController.ModuleStartup(Engine);

   /* Register component factories */
   componentRegistry.RegisterComponentFactory<CameraComponent>(&componentFactory_Camera);
   componentRegistry.RegisterComponentFactory<DebugShapeComponent>(
     &componentFactory_DebugShape);
   componentRegistry.RegisterComponentFactory<PhysXActorComponent>(
     &componentFactory_PhysXActor);
   componentRegistry.RegisterComponentFactory<PhysXControllerComponent>(
     &componentFactory_PhysXController);

   return CT_SUCCESS;
}

ctResults ctHoneybell::Scene::Shutdown() {
   ZoneScoped;
   /* Shutdown component factories */
   componentFactory_Camera.ModuleShutdown();
   componentFactory_DebugShape.ModuleShutdown();
   componentFactory_PhysXActor.ModuleShutdown();
   componentFactory_PhysXController.ModuleShutdown();
   ctAlignedFree(physMemory);
   return CT_SUCCESS;
}

void ctHoneybell::Scene::NextFrame() {
   ZoneScoped;
   /* Draw debug shapes */
   componentFactory_DebugShape.Im3dDrawAll();
}

void ctHoneybell::Scene::Simulate(double deltaTime, ctJobSystem* pJobSystem) {
   ZoneScoped;

   /* Simulation ticks */
   tickTimer += deltaTime;
   ctStopwatch tickLapTimer = ctStopwatch();
   double totalTickTime = 0.0;
   while (tickTimer >= tickInterval) {
      /* Setup tick context */
      struct TickContext tickCtx = TickContext();
      tickCtx.deltaTime = tickInterval;
      tickCtx.gravity = globalGravity;

      /* Tick Parallel */
      parallelJobData.Resize(toys_TickParallel.Count());
      for (int i = 0; i < toys_TickParallel.Count(); i++) {
         ToyBase* pToy = toys_TickParallel[i];
         if (pToy) {
            parallelJobData[i] = {tickCtx, pToy};
            pJobSystem->PushJob(doParallelTickJob, &parallelJobData[i]);
         }
      }
      if (!parallelJobData.isEmpty()) { pJobSystem->RunAllJobs(); }

      /* Tick Serial */
      for (int i = 0; i < toys_TickSerial.Count(); i++) {
         ToyBase* pToy = toys_TickSerial[i];
         if (pToy) { pToy->OnTickSerial(tickCtx); }
      }

#if CITRUS_PHYSX
      /*Todo: scratch buffer*/
      if (pPxScene) {
         pPxScene->simulate(
           (physx::PxReal)tickInterval, NULL, physMemory, PHYSX_SCRATCH_MEMORY_AMOUNT);
         pPxScene->fetchResults(true); /* Todo: investigate better fetch */
      }
#endif

      /* Todo: Handle deletions */

      /* Handle tick budget issues */
      tickLapTimer.NextLap();
      const double tickTime = tickLapTimer.GetDeltaTime();
      totalTickTime += tickTime;
      if (tickTime > tickInterval) {
         tickTimer -= tickTime;
         ctDebugWarning(
           "Tick is over budget! %lf>%lf: Throttling to avoid deathspiral...",
           tickTime,
           tickInterval);
         if (tickTime > 0.02) {
            ctDebugWarning("Tick hitch detected... Bailed out!");
            break;
         }
      } else {
         tickTimer -= tickInterval;
         if (tickTimer < 0.0) { tickTimer = 0.0; }
      }
   }

   /* Frame updates */
   FrameUpdateContext updateCtx = FrameUpdateContext();
   updateCtx.deltaTime = deltaTime;
   updateCtx.gravity = globalGravity;
   for (int i = 0; i < toys_FrameUpdate.Count(); i++) {
      ToyBase* pToy = toys_FrameUpdate[i];
      if (pToy) { pToy->OnFrameUpdate(updateCtx); }
   }
}

void ctHoneybell::Scene::ClearScene() {
   ZoneScoped;
   for (size_t i = 0; i < toysLinear.Count(); i++) {
      if (toysLinear[i]) { delete toysLinear[i]; }
   }
   toyHandleManager = ctHandleManager();
   toysLinear.Clear();
   toys.Clear();
   toys_FrameUpdate.Clear();
   toys_TickParallel.Clear();
   toys_TickSerial.Clear();
}

ctResults ctHoneybell::Scene::SpawnToy(const char* prefabPath,
                                       ctTransform& transform,
                                       const char* message) {
   ZoneScoped;
   ctWADAsset* pWadAsset = Engine->AssetManager->GetWADAsset(prefabPath);
   if (!pWadAsset) {
      ctDebugWarning("Failed to spawn %s at (%f,%f,%f)",
                     prefabPath,
                     transform.position.x,
                     transform.position.y,
                     transform.position.z);
      SpawnErrorToy(transform);
      return CT_FAILURE_FILE_NOT_FOUND;
   }

   char* pTypePath = NULL;
   int32_t typePathSize = 0;
   ctStringUtf8 sanitizedPath;
   ctWADFindLump(&pWadAsset->wadReader, "TOYTYPE", (void**)&pTypePath, &typePathSize);
   if (!pTypePath) {
      ctDebugWarning("Failed to spawn %s: No toy type!", prefabPath);
      SpawnErrorToy(transform);
      return CT_FAILURE_CORRUPTED_CONTENTS;
   }
   sanitizedPath = ctStringUtf8(pTypePath, typePathSize);

   ctHoneybell::SpawnData spawnData = ctHoneybell::SpawnData();
   spawnData.transform = transform;
   spawnData.message = message;
   ctHoneybell::PrefabData prefabData = ctHoneybell::PrefabData();
   prefabData.wadReader = pWadAsset->wadReader;
   ctHoneybell::ConstructContext constructCtx = ctHoneybell::ConstructContext();
   constructCtx.pOwningScene = this;
   constructCtx.pComponentRegistry = &componentRegistry;
   constructCtx.pPhysics = Engine->PhysXIntegration->pPhysics;
   constructCtx.spawn = spawnData;
   constructCtx.prefab = prefabData;
   constructCtx.typePath = sanitizedPath.CStr();
   ctHoneybell::ToyBase* newToy = pToyRegistry->NewToy(constructCtx);
   if (newToy) {
      newToy->_SetIdentifier(_RegisterToy(newToy));
      toysLinear.Append(newToy);
   }
   pWadAsset->Dereferene();
   return CT_SUCCESS;
}

ctResults ctHoneybell::Scene::SpawnInternalToy(const char* toyType,
                                               ctTransform& transform,
                                               const char* message) {
   ZoneScoped;
   ctHoneybell::SpawnData spawnData = ctHoneybell::SpawnData();
   spawnData.transform = transform;
   spawnData.message = message;
   ctHoneybell::PrefabData prefabData = ctHoneybell::PrefabData();
   ctHoneybell::ConstructContext constructCtx = ctHoneybell::ConstructContext();
   constructCtx.pOwningScene = this;
   constructCtx.pComponentRegistry = &componentRegistry;
   constructCtx.pPhysics = Engine->PhysXIntegration->pPhysics;
   constructCtx.spawn = spawnData;
   constructCtx.prefab = prefabData;
   constructCtx.typePath = toyType;
   ctHoneybell::ToyBase* newToy = pToyRegistry->NewToy(constructCtx);
   if (newToy) {
      newToy->_SetIdentifier(_RegisterToy(newToy));
      toysLinear.Append(newToy);
   }
   return CT_SUCCESS;
}

ctResults ctHoneybell::Scene::SpawnErrorToy(ctTransform& transform) {
   return SpawnInternalToy("citrus/testShape", transform, "");
}

int ctScriptApi::Honeybell::spawnToy(ctScriptTypedLightData* ldata,
                                     const char* path,
                                     float x,
                                     float y,
                                     float z,
                                     float yaw,
                                     float pitch,
                                     float roll,
                                     float scale,
                                     const char* message) {
   ctTransform transform =
     ctTransform(ctVec3(x, y, z), ctQuatYawPitchRoll(yaw, pitch, roll), ctVec3(scale));
   ((ctHoneybell::Scene*)(ldata->ptr))->SpawnToy(path, transform, message);
   return 0;
}

int ctScriptApi::Honeybell::spawnInternalToy(ctScriptTypedLightData* ldata,
                                             const char* type,
                                             float x,
                                             float y,
                                             float z,
                                             float yaw,
                                             float pitch,
                                             float roll,
                                             float scale,
                                             const char* message) {
   ctTransform transform = ctTransform(ctVec3(x, y, z), ctQuatYawPitchRoll(yaw, pitch, roll), ctVec3(scale));
   ((ctHoneybell::Scene*)(ldata->ptr))->SpawnInternalToy(type, transform, message);
   return 0;
}