/*
   Copyright 2022 MacKenzie Strand

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

#include "Toy.hpp"
#include "Component.hpp"
#include "Scene.hpp"

ctHoneybell::ToyBase::ToyBase(ConstructContext& ctx) {
   identifier = 0;
   transform.position = ctx.spawn.transform.position;
   transform.rotation = ctx.spawn.transform.rotation;
   transform.scale = ctx.spawn.transform.scale;
   aabb = ctBoundBox();
   pFirstComponent = NULL;
}

ctHoneybell::ToyBase::~ToyBase() {
   /* Auto destruct all components */
   ComponentBase* nextComponent = pFirstComponent;
   if (!nextComponent) { return; }
   while (nextComponent->pNextSiblingComponent) {
      ComponentBase* lastComponent = nextComponent;
      nextComponent = nextComponent->pNextSiblingComponent;
      delete lastComponent;
   }
   delete nextComponent;
}

ctResults ctHoneybell::ToyBase::OnBegin(BeginContext& ctx) {
   return CT_SUCCESS;
}

ctResults ctHoneybell::ToyBase::OnTickSerial(TickContext& ctx) {
   return CT_SUCCESS;
}

ctResults ctHoneybell::ToyBase::OnTickParallel(TickContext& ctx) {
   return CT_SUCCESS;
}

ctResults ctHoneybell::ToyBase::OnFrameUpdate(FrameUpdateContext& ctx) {
   return CT_SUCCESS;
}

ctResults ctHoneybell::ToyBase::OnTryPossess(PossessionContext& ctx) {
   return CT_FAILURE_INACCESSIBLE;
}

/* Default point of view handling */
ctResults ctHoneybell::ToyBase::GetPointOfView(PointOfViewContext& ctx) {
   ctTransform transform = GetWorldTransform();
   float radius = ctBoundSphere(GetAABB()).radius;
   if (radius < 0.0f) { radius = 1.0f; }
   ctx.cameraInfo.rotation = transform.rotation;
   ctx.cameraInfo.position = transform.position +
                             (transform.rotation.getBack() * radius * 10.0f) +
                             transform.rotation.getUp() * radius * 2.0f;
   return CT_SUCCESS;
}

void ctHoneybell::ToyBase::SetWorldTransform(ctTransform v) {
   transform = v;
}

void ctHoneybell::ToyBase::CopyComponentTransform(ComponentBase* pComponent) {
   if (!pComponent) { return; }
   SetWorldTransform(pComponent->GetWorldTransform());
}

ctTransform ctHoneybell::ToyBase::GetWorldTransform() {
   return transform;
}

ctBoundBox ctHoneybell::ToyBase::GetAABB() {
   return aabb;
}

void ctHoneybell::ToyBase::SetAABB(ctBoundBox v) {
   aabb = v;
}

void ctHoneybell::ToyBase::BeginComponents(BeginContext& ctx) {
   if (!pFirstComponent) { return; }
   ComponentBase* nextComponent = pFirstComponent;
   while (nextComponent->pNextSiblingComponent) {
      nextComponent->Begin(ctx);
      nextComponent = nextComponent->pNextSiblingComponent;
   }
   nextComponent->Begin(ctx);
}

ctHandle ctHoneybell::ToyBase::GetIdentifier() {
   return identifier;
}

ctResults ctHoneybell::ToyBase::_CallSignal(SignalContext& ctx) {
   return CT_FAILURE_INACCESSIBLE;
}

void ctHoneybell::ToyBase::_RegisterComponent(ComponentBase* v) {
   if (!pFirstComponent) {
      pFirstComponent = v;
   } else {
      ComponentBase* nextComponent = pFirstComponent;
      while (nextComponent->pNextSiblingComponent) {
         nextComponent = nextComponent->pNextSiblingComponent;
      }
      nextComponent->pNextSiblingComponent = v;
   }
}

void ctHoneybell::ToyBase::_SetIdentifier(ctHandle hndl) {
   identifier = hndl;
}

ctHoneybell::ToyBase* ctHoneybell::ToyTypeRegistry::NewToy(ConstructContext& ctx) {
   ZoneScoped;
   uint64_t typeHash = ctXXHash64(ctx.typePath);
   ctAssert(typeHash != 0);
   ToyNewFunction* pCallback = _callbacks.FindPtr(typeHash);
   if (!pCallback) {
      ctDebugError("Failed to spawn toy of type: %s", ctx.typePath);
      return NULL;
   }
   return (*pCallback)(ctx);
}

ctResults ctHoneybell::ToyTypeRegistry::RegisterToyType(const char* typePath,
                                                        ToyNewFunction toyNewFunction) {
   ZoneScoped;
   uint64_t typeHash = ctXXHash64(typePath);
   ctAssert(typeHash != 0);
   if (!_callbacks.Insert(typeHash, toyNewFunction)) { return CT_FAILURE_UNKNOWN; }
   return CT_SUCCESS;
}