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
#include "Component.hpp"

/* All component types */
#include "components/CameraComponent.hpp"
#include "components/DebugShapeComponent.hpp"
#include "components/PhysXActorComponent.hpp"
#include "components/PhysXControllerComponent.hpp"

#if CITRUS_PHYSX
#include "PxScene.h"
#endif

class ctEngineCore;

namespace ctHoneybell {

class CT_API Scene : public ctModuleBase {
public:
   Scene();
   ~Scene();

   virtual ctResults Startup();
   virtual ctResults Shutdown();

   void ClearScene();
   ctResults SpawnToy(const char* prefabPath,
                      ctTransform& transform = ctTransform(),
                      const char* message = NULL);
   ctResults SpawnInternalToy(const char* toyType,
                              ctTransform& transform = ctTransform(),
                              const char* message = NULL);
   ctResults SpawnErrorToy(ctTransform& transform);

   void Simulate(double deltaTime, ctJobSystem* pJobSystem);
   void NextFrame();
   double tickInterval = 1.0 / 60;

   /* -------- Don't call these manually --------*/
   ctHandle _RegisterToy(ToyBase* toy);
   void _UnregisterToy(ctHandle hndl);

   ComponentRegistry componentRegistry;
   ToyTypeRegistry* pToyRegistry;

#if CITRUS_PHYSX
   physx::PxScene* pPxScene;
   void* physMemory;
#endif
   class ctEngineCore* Engine;

private:
   ctVec3 globalGravity;

   ctHandleManager toyHandleManager;
   ctHashTable<ToyBase*, ctHandle> toys;
   ctDynamicArray<ToyBase*> toysLinear;

   ctDynamicArray<ToyBase*> toys_TickSerial;
   ctDynamicArray<ToyBase*> toys_TickParallel;
   ctDynamicArray<ToyBase*> toys_FrameUpdate;

   /* Component factories */
   CameraComponentFactory componentFactory_Camera;
   DebugShapeComponentFactory componentFactory_DebugShape;
   PhysXActorComponentFactory componentFactory_PhysXActor;
   PhysXControllerComponentFactory componentFactory_PhysXController;

   double tickTimer;

   struct parallelTickJobData {
      ctHoneybell::TickContext tickCtx;
      ctHoneybell::ToyBase* pToy;
   };
   ctDynamicArray<parallelTickJobData> parallelJobData;
};
}