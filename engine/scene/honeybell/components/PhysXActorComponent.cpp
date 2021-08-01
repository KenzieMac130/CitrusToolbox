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
#include "PhysXActorComponent.hpp"

#include "PxScene.h"

ctHoneybell::PhysXActorComponent::PhysXActorComponent(
  class ComponentFactoryBase* _factory, class ToyBase* _toy) :
    ComponentBase::ComponentBase(_factory, _toy) {
   pPxRigidActor = NULL;
}

ctHoneybell::PhysXActorComponent::~PhysXActorComponent() {
   PhysXActorComponentFactory* pConcreteFactory = (PhysXActorComponentFactory*)(pFactory);
   if (pConcreteFactory) { pConcreteFactory->pPxScene->removeActor(*pPxRigidActor); }
   if (pPxRigidActor) { pPxRigidActor->release(); }
   for (size_t i = 0; i < PxMaterialStorage.Count(); i++) {
      if (PxMaterialStorage[i]) { PxMaterialStorage[i]->release(); }
   }
   for (size_t i = 0; i < PxShapeStorage.Count(); i++) {
      if (PxShapeStorage[i]) { PxShapeStorage[i]->release(); }
   }
}

ctHoneybell::ComponentBase*
ctHoneybell::PhysXActorComponentFactory::NewComponent(ToyBase* _owner) {
   return new PhysXActorComponent(this, _owner);
}

ctResults ctHoneybell::PhysXActorComponent::AddToScene() {
   if (!pFactory) {
      ctDebugError("PhysXActorComponent: Does not have a factory!");
      return CT_FAILURE_MODULE_NOT_INITIALIZED;
   }
   if (!pPxRigidActor) {
      ctDebugError(
        "PhysXActorComponent: Attempted to add pPxRigidActor of NULL to the scene!");
      return CT_FAILURE_INVALID_PARAMETER;
   }
   PhysXActorComponentFactory* pConcreteFactory = (PhysXActorComponentFactory*)(pFactory);
   if (!pConcreteFactory->pPxScene) { return CT_FAILURE_MODULE_NOT_INITIALIZED; }
   pConcreteFactory->pPxScene->addActor(*pPxRigidActor);
   return CT_SUCCESS;
}

bool ctHoneybell::PhysXActorComponent::hasTransform() const {
   return true;
}

ctTransform ctHoneybell::PhysXActorComponent::GetWorldTransform() {
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