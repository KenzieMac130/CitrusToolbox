/*
   Copyright 2023 MacKenzie Strand

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
#include "../core/ModuleBase.hpp"

typedef uint32_t ctKinnowEntity;

typedef void (*ctKinnowDestructorFunc)(void* pComponentData, void* pUserData);

/* factories defines the creation of components */
class CT_API ctKinnowComponentFactory {
   public:
   ctKinnowComponentFactory(size_t dataSize = 0);
   ~ctKinnowComponentFactory();

   virtual ctResults OnRegister(class ctKinnowWorld& world);
   virtual ctResults OnUnregister(class ctKinnowWorld& world);
   virtual void ComponentDestructor(void* pComponent);
};

class CT_API ctKinnowSpawnInfo {
   /* todo */
};

class CT_API ctKinnowScheduleContext {
   /* todo */
   // define filter
   // define run after
   /* expression:
   *: optional modifier (needs a null check)
   r: fetch component for read
   w: fetch component for write
   rw: fetch component for read and write
   f: uses a filter function to approve initial check (add to anything)
   m: random access manual fetch (requires r/w/rw, don't add to args)
   #: tag only (don't add to args)
   !: tag only exclude (don't add to args)
   "...": name of component
   i: arg is entity id */
};

class CT_API ctKinnowExecuteContext {
   /* todo */
   /* job system */
};

/* systems can process components */
class CT_API ctKinnowSystem {
public:
   virtual ctResults OnRegister(class ctKinnowWorld& world);
   virtual ctResults OnUnregister(class ctKinnowWorld& world);
   virtual ctResults DefineDependencies(class ctKinnowWorld& world, ctKinnowScheduleContext& context);
   virtual ctResults Execute(ctKinnowExecuteContext& context);
};

/* concepts can instance objects in the world */
class CT_API ctKinnowConcept {
   public:
   virtual ctResults OnRegister(class ctKinnowWorld& world);
   virtual ctResults OnUnregister(class ctKinnowWorld& world);
   virtual ctResults Spawn(class ctKinnowWorld& world, const ctKinnowSpawnInfo info);
};

/* contains the simulation */
class CT_API ctKinnowWorld : public ctModuleBase {
      public:
   ctKinnowWorld(size_t entityCountReserve, size_t componentMemPoolReserve);
   ~ctKinnowWorld();

   ctResults RegisterComponentFactory(const char* name, ctKinnowComponentFactory* pFactory);
   ctResults UnregisterComponentFactory(const char* name);

   ctResults RegisterSystem(const char* name, ctKinnowConcept* pConcept);
   ctResults UnregisterSystem(const char* name, ctKinnowConcept* pConcept);

   ctResults RegisterConcept(const char* name, ctKinnowConcept* pConcept);
   ctResults UnregisterConcept(const char* name, ctKinnowConcept* pConcept);

   ctResults CreateEntity(ctKinnowEntity& entity);
   ctResults DestroyEntity(ctKinnowEntity entity);

   ctResults AddComponent(ctKinnowEntity entity, const char* name, void* pData = NULL);
   ctResults RemoveComponent(ctKinnowEntity entity, const char* name);

   /* dispatches many threads to tackle simulation */
   ctResults Simulate(double deltaTime);
};