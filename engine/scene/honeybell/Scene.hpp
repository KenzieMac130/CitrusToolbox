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

#include "core/JobSystem.hpp"

#include "scripting/api/HoneybellScript.hpp"
#include "Toy.hpp"

#if CITRUS_PHYSX
#include "PxScene.h"
#include "characterkinematic/PxControllerManager.h"
#endif

#include "components/DebugShapeComponent.hpp"

namespace ctHoneybell {

class CT_API Scene {
public:
   Scene();
   ~Scene();

   ctEngineCore* Engine;
   virtual ctResults Startup();
   virtual ctResults Shutdown();

   void ClearScene();
   ctResults SpawnToy(const char* toyType,
                      ctTransform& transform = ctTransform(),
                      const char* message = NULL,
                      const char* prefabWadPath = NULL,
                      ctHandle* pResultHandle = ctHandle());
   ToyBase* FindToyByHandle(ctHandle handle);

   void Simulate(double deltaTime, ctJobSystem* pJobSystem);
   void NextFrame();
   double tickInterval = 1.0 / 60;

   /* -------- Don't call these manually --------*/
   ctHandle _RegisterToy(ToyBase* toy);
   void _UnregisterToy(ctHandle hndl);

   ToyTypeRegistry* pToyRegistry;
   SignalManager signalManager;

#if CITRUS_PHYSX
   physx::PxScene* pPxScene;
   physx::PxControllerManager* pPxControllerManager;
   void* physMemory;
#endif

   DebugShapeManager debugShapes;

private:
   ctVec3 globalGravity;

   ctHandleManager toyHandleManager;
   ctHashTable<ToyBase*, ctHandle> toys;
   ctDynamicArray<ToyBase*> toysLinear;

   ctDynamicArray<ToyBase*> toys_TickSerial;
   ctDynamicArray<ToyBase*> toys_TickParallel;
   ctDynamicArray<ToyBase*> toys_FrameUpdate;

   double tickTimer;

   struct parallelTickJobData {
      ctHoneybell::TickContext tickCtx;
      ctHoneybell::ToyBase* pToy;
   };
   ctDynamicArray<parallelTickJobData> parallelJobData;
};
}