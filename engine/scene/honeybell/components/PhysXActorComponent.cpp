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

#include "PhysXActorComponent.hpp"

#include "PxScene.h"

ctHoneybell::PhysXActorComponent::PhysXActorComponent(ConstructContext& ctx,
                                                      class ToyBase* _toy) :
    ComponentBase::ComponentBase(ctx, _toy) {
   pPxRigidActor = NULL;
}

ctHoneybell::PhysXActorComponent::~PhysXActorComponent() {
   if (pPxRigidActor) {
      if (pPxRigidActor->getScene()) {
         pPxRigidActor->getScene()->removeActor(*pPxRigidActor);
      }
      pPxRigidActor->release();
   }
   for (size_t i = 0; i < PxMaterialStorage.Count(); i++) {
      if (PxMaterialStorage[i]) { PxMaterialStorage[i]->release(); }
   }
   for (size_t i = 0; i < PxShapeStorage.Count(); i++) {
      if (PxShapeStorage[i]) { PxShapeStorage[i]->release(); }
   }
}

ctResults ctHoneybell::PhysXActorComponent::Begin(BeginContext& beginCtx) {
   if (!pPxRigidActor) {
      ctDebugError(
        "PhysXActorComponent: Attempted to add pPxRigidActor of NULL to the scene!");
      return CT_FAILURE_INVALID_PARAMETER;
   }
   if (!beginCtx.pPxScene) { return CT_FAILURE_MODULE_NOT_INITIALIZED; }
   beginCtx.pPxScene->addActor(*pPxRigidActor);
   return CT_SUCCESS;
}

bool ctHoneybell::PhysXActorComponent::hasTransform() const {
   return true;
}

ctTransform ctHoneybell::PhysXActorComponent::GetWorldTransform() const {
   if (!pPxRigidActor) { return ctTransform(); }
   return ctTransformFromPx(pPxRigidActor->getGlobalPose());
}

void ctHoneybell::PhysXActorComponent::SetWorldTransform(ctTransform v) {
   if (!pPxRigidActor) { return; }
   return pPxRigidActor->setGlobalPose(ctTransformToPx(v));
}

ctBoundBox ctHoneybell::PhysXActorComponent::GetWorldBounds() {
   return ctBoundBoxFromPx(pPxRigidActor->getWorldBounds());
}

const char* ctHoneybell::PhysXActorComponent::GetTypeName() {
   return "PhysXActorComponent";
}
