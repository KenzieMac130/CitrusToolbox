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

//#include "asset/types/WADAsset.hpp"
#include "formats/wad/prototypes/Header.h"

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
   ctHandle toyHandle = toyHandleManager.GetNewHandle();
   CT_PANIC_UNTRUE(toys.Insert(toyHandle, toy),
                   "Internal scene error: Failed to add toy");
   BeginContext ctx = BeginContext();
   ctx.pOwningScene = this;
   ctx.pSignalManager = &signalManager;
   ctx.pPxScene = pPxScene;
   ctx.pPxControllerManager = pPxControllerManager;
   toy->_SetIdentifier(toyHandle);
   toy->OnBegin(ctx);
   if (ctx.canTickSerial) { toys_TickSerial.Append(toy); }
   if (ctx.canTickParallel) { toys_TickParallel.Append(toy); }
   if (ctx.canFrameUpdate) { toys_FrameUpdate.Append(toy); }
   if (ctx.canHandleSignals) { signalManager.AddToy(toy); }
   return toyHandle;
}

void ctHoneybell::Scene::_UnregisterToy(ctHandle hndl) {
   ZoneScoped;
   ToyBase** toy = toys.FindPtr(hndl);
   if (toy) {
      toys_TickSerial.Remove(*toy);
      toys_TickParallel.Remove(*toy);
      toys_FrameUpdate.Remove(*toy);
      signalManager.RemoveToy(*toy);
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
   pPxControllerManager = PxCreateControllerManager(*pPxScene);
#endif
   return CT_SUCCESS;
}

ctResults ctHoneybell::Scene::Shutdown() {
   ZoneScoped;
   ClearScene();
   pPxControllerManager->release();
   pPxScene->release();
   ctAlignedFree(physMemory);
   return CT_SUCCESS;
}

void ctHoneybell::Scene::NextFrame() {
   ZoneScoped;
   debugShapes.Im3dDrawAll();
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
      tickCtx.pSignalManager = &signalManager;

      /* Tick Parallel */
      parallelJobData.Resize(toys_TickParallel.Count());
      for (int i = 0; i < toys_TickParallel.Count(); i++) {
         ToyBase* pToy = toys_TickParallel[i];
         if (pToy) {
            parallelJobData[i] = {tickCtx, pToy};
            //pJobSystem->PushJob(doParallelTickJob, &parallelJobData[i]);
         }
      }
      //if (!parallelJobData.isEmpty()) { pJobSystem->RunAllJobs(); }

      /* Tick Serial */
      for (int i = 0; i < toys_TickSerial.Count(); i++) {
         ToyBase* pToy = toys_TickSerial[i];
         if (pToy) { pToy->OnTickSerial(tickCtx); }
      }

      signalManager.DispatchSignals();

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
   updateCtx.pSignalManager = &signalManager;
   for (int i = 0; i < toys_FrameUpdate.Count(); i++) {
      ToyBase* pToy = toys_FrameUpdate[i];
      if (pToy) { pToy->OnFrameUpdate(updateCtx); }
   }
   signalManager.DispatchSignals();
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
   debugShapes.shapes.Clear();
}

ctResults ctHoneybell::Scene::SpawnToy(const char* toyType,
                                       ctTransform& transform,
                                       const char* message,
                                       const char* prefabWadPath,
                                       ctHandle* pResultHandle) {
   ZoneScoped;

   //ctWADAsset* pWadAsset = NULL;
   //if (prefabWadPath && !ctCStrEql(prefabWadPath, "")) {
   //   // pWadAsset = Engine->AssetManager->GetWADAsset(prefabWadPath);
   //   // pWadAsset->WaitForLoad();
   //}

   //if (pWadAsset) {
   //   ctWADProtoHeader* pWadHeader;
   //   ctWADFindLump(&pWadAsset->wadReader, "CITRUS", (void**)&pWadHeader, NULL);
   //   if (!pWadHeader) {
   //      ctDebugWarning("Failed to spawn %s: No citrus header!", prefabWadPath);
   //      return CT_FAILURE_CORRUPTED_CONTENTS;
   //   }
   //}

   ctHoneybell::SpawnData spawnData = ctHoneybell::SpawnData();
   spawnData.transform = transform;
   spawnData.message = message;
   ctHoneybell::PrefabData prefabData = ctHoneybell::PrefabData();
   //if (pWadAsset) { prefabData.wadReader = pWadAsset->wadReader; }
   ctHoneybell::ConstructContext constructCtx = ctHoneybell::ConstructContext();
   constructCtx.pOwningScene = this;
   constructCtx.pPhysics = Engine->PhysXIntegration->pPhysics;
   constructCtx.spawn = spawnData;
   constructCtx.prefab = prefabData;
   constructCtx.typePath = toyType;
   ctHoneybell::ToyBase* newToy = pToyRegistry->NewToy(constructCtx);
   if (newToy) {
      ctHandle hndl = _RegisterToy(newToy);
      if (pResultHandle) { *pResultHandle = hndl; }
      toysLinear.Append(newToy);
   }
   // if (pWadAsset) { pWadAsset->Dereferene(); }
   return CT_SUCCESS;
}

ctHoneybell::ToyBase* ctHoneybell::Scene::FindToyByHandle(ctHandle handle) {
   ToyBase** ppToy = toys.FindPtr(handle);
   if (ppToy) { return *ppToy; }
   return NULL;
}

int ctScriptApi::Honeybell::spawnToy(ctScriptTypedLightData* ldata,
                                     const char* type,
                                     float x,
                                     float y,
                                     float z,
                                     float yaw,
                                     float pitch,
                                     float roll,
                                     float scale,
                                     const char* message,
                                     const char* prefabPath) {
   ctTransform transform =
     ctTransform(ctVec3(x, y, z), ctQuatYawPitchRoll(yaw, pitch, roll), ctVec3(scale));
   ((ctHoneybell::Scene*)(ldata->ptr))->SpawnToy(type, transform, message, prefabPath);
   return 0;
}