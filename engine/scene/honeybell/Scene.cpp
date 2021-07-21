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
      ctHoneybell::TickParallelContext tickCtx;
      ctHoneybell::ToyBase* pToy;
   };
   jobData* job = (jobData*)pData;
   job->pToy->OnTickParallel(job->tickCtx);
}

ctResults ctHoneybell::Scene::Startup() {
   ZoneScoped;
   /* Startup component factories */
   componentFactory_Camera.ModuleStartup(Engine);
   /* Register component factories */
   componentRegistry.RegisterComponentFactory<CameraComponent>(&componentFactory_Camera);
   return CT_SUCCESS;
}

ctResults ctHoneybell::Scene::Shutdown() {
   ZoneScoped;
   /* Shutdown component factories */
   componentFactory_Camera.Shutdown();
   return CT_SUCCESS;
}

void ctHoneybell::Scene::Update(double deltaTime, ctJobSystem* pJobSystem) {
   ZoneScoped;
   /* Simulation ticks */
   tickTimer += deltaTime;
   ctStopwatch tickLapTimer = ctStopwatch();
   double totalTickTime = 0.0;
   while (tickTimer >= tickInterval) {
      /* Setup tick context */
      struct TickContext tickCtx = TickContext();
      tickCtx.deltaTime = tickInterval;

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

      /* Todo: Handle deletions */

      /* Handle tick budget issues */
      tickLapTimer.NextLap();
      const double tickTime = tickLapTimer.GetDeltaTime();
      totalTickTime += tickTime;
      if (tickTime > tickInterval) {
         tickTimer -= tickTime;
         ctDebugWarning("Tick is over budget! %lf>%lf: Throttling to avoid deathspiral...", tickTime, tickInterval);
         if (tickTime > 0.10) {
            ctDebugWarning("Tick hitch detected... Bailed out!");
            break;
         }
      } else {
         tickTimer -= tickInterval;
      }
   }

   /* Frame updates */
   FrameUpdateContext updateCtx = FrameUpdateContext();
   updateCtx.deltaTime = deltaTime;
   for (int i = 0; i < toys_FrameUpdate.Count(); i++) {
      ToyBase* pToy = toys_FrameUpdate[i];
      if (pToy) { pToy->OnFrameUpdate(updateCtx); }
   }
}