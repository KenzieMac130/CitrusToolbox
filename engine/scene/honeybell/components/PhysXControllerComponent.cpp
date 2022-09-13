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

#include "PhysXControllerComponent.hpp"

#include "PxScene.h"

ctHoneybell::PhysXControllerComponent::PhysXControllerComponent(ConstructContext& ctx,
                                                                class ToyBase* _toy) :
    ComponentBase::ComponentBase(ctx, _toy) {
   pPxController = NULL;
   pPxMaterialStorage = NULL;
   rotation = ctQuat();
}

ctHoneybell::PhysXControllerComponent::~PhysXControllerComponent() {
   if (pPxController) { pPxController->release(); }
   if (pPxMaterialStorage) { pPxMaterialStorage->release(); }
}

bool ctHoneybell::PhysXControllerComponent::hasTransform() const {
   return true;
}

ctTransform ctHoneybell::PhysXControllerComponent::GetWorldTransform() const {
   if (!pPxController) { return ctTransform(); }
   return ctTransform(ctVec3FromPxExt(pPxController->getPosition()), rotation);
}

void ctHoneybell::PhysXControllerComponent::SetWorldTransform(ctTransform& v) {
   if (!pPxController) { return; }
   rotation = v.rotation;
   pPxController->setPosition(ctVec3ToPxExt(v.position));
}

ctResults ctHoneybell::PhysXControllerComponent::Begin(BeginContext& ctx) {
   if (!pPxMaterialStorage) {
      pPxMaterialStorage = ctx.pPxScene->getPhysics().createMaterial(0.5f, 0.5f, 0.6f);
   }
   ctTransform xform = GetWorldTransform();
   PxCapsuleControllerDesc desc = PxCapsuleControllerDesc();
   desc.height = 1.0f;
   desc.radius = 0.25f;

   desc.stepOffset = 0.1f;
   desc.position = ctVec3ToPxExt(xform.position);
   desc.material = pPxMaterialStorage;
   desc.upDirection = ctVec3ToPx(rotation.getUp());
   pPxController = ctx.pPxControllerManager->createController(desc);
   if (!pPxController) { return CT_FAILURE_INVALID_PARAMETER; }
   return CT_SUCCESS;
}

ctBoundBox ctHoneybell::PhysXControllerComponent::GetWorldBounds() {
   PxRigidDynamic* actor = pPxController->getActor();
   if (!actor) { return ctBoundBox(); }
   return ctBoundBoxFromPx(actor->getWorldBounds());
}

const char* ctHoneybell::PhysXControllerComponent::GetTypeName() {
   return "PhysXControllerComponent";
}
