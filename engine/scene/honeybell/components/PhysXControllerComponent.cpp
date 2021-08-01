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

#include "CameraComponent.hpp"
#include "PhysXControllerComponent.hpp"

#include "PxScene.h"

ctHoneybell::PhysXControllerComponent::PhysXControllerComponent(
  class ComponentFactoryBase* _factory, class ToyBase* _toy) :
    ComponentBase::ComponentBase(_factory, _toy) {
   pPxController = NULL;
   pPxMaterialStorage = NULL;
}

ctHoneybell::PhysXControllerComponent::~PhysXControllerComponent() {
   if (pPxController) { pPxController->release(); }
   if (pPxMaterialStorage) { pPxMaterialStorage->release(); }
}

ctResults ctHoneybell::PhysXControllerComponentFactory::Startup() {
   if (pPxScene) {
      pControllerManager = PxCreateControllerManager(*pPxScene);
      return CT_SUCCESS;
   }
   return CT_FAILURE_MODULE_NOT_INITIALIZED;
}

ctResults ctHoneybell::PhysXControllerComponentFactory::Shutdown() {
   if (pControllerManager) { pControllerManager->release(); }
   return CT_SUCCESS;
}

ctHoneybell::ComponentBase*
ctHoneybell::PhysXControllerComponentFactory::NewComponent(ToyBase* _owner) {
   return new PhysXControllerComponent(this, _owner);
}

bool ctHoneybell::PhysXControllerComponent::hasTransform() const {
   return true;
}

ctTransform ctHoneybell::PhysXControllerComponent::GetWorldTransform() {
   if (!pPxController) { return ctTransform(); }
   return ctTransform(ctVec3FromPxExt(pPxController->getPosition());
}

void ctHoneybell::PhysXControllerComponent::SetWorldTransform(ctTransform v) {
   if (!pPxController) { return; }
   pPxController->setPosition(ctVec3ToPxExt(v.position));
}

ctResults ctHoneybell::PhysXControllerComponent::InitController(PxControllerDesc* pDesc) {
   if (!pFactory) { return CT_FAILURE_MODULE_NOT_INITIALIZED; }
   const PhysXControllerComponentFactory* ctrlFactory =
     (PhysXControllerComponentFactory*)pFactory;
   if (!pDesc) {
      if (!pPxMaterialStorage) {
         pPxMaterialStorage =
           ctrlFactory->pPxScene->getPhysics().createMaterial(0.5f, 0.5f, 0.6f);
      }
      ctTransform xform = GetWorldTransform();
      PxCapsuleControllerDesc desc = PxCapsuleControllerDesc();
      desc.height = 1.0f;
      desc.radius = 0.25f;
      desc.stepOffset = 0.1f;
      desc.position = ctVec3ToPxExt(xform.position);
      desc.material = pPxMaterialStorage;
      pPxController = ctrlFactory->pControllerManager->createController(desc);
   } else {
      pPxController = ctrlFactory->pControllerManager->createController(*pDesc);
   }
   if (!pPxController) { return CT_FAILURE_INVALID_PARAMETER; }
   return CT_SUCCESS;
}

ctBoundBox ctHoneybell::PhysXControllerComponent::GetWorldBounds() {
   PxRigidDynamic* actor = pPxController->getActor();
   if (!actor) { return ctBoundBox(); }
   return ctBoundBoxFromPx(actor->getWorldBounds());
}