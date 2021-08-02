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

#include "../Component.hpp"

#include "PxPhysicsAPI.h"

using namespace physx;

namespace ctHoneybell {

class CT_API PhysXActorComponent : public ComponentBase {
public:
   PhysXActorComponent(class ComponentFactoryBase* _factory, class ToyBase* _toy);
   ~PhysXActorComponent();
   ctResults AddToScene();

   virtual bool hasTransform() const;
   virtual ctTransform GetWorldTransform();
   virtual void SetWorldTransform(ctTransform v);
   virtual ctBoundBox GetWorldBounds();

   PxRigidActor* pPxRigidActor;
   ctStaticArray<PxMaterial*, 32> PxMaterialStorage;
   ctStaticArray<PxShape*, 32> PxShapeStorage;
};

/* Boilerplate */
class CT_API PhysXActorComponentFactory : public ComponentFactoryBase {
public:
   virtual ComponentBase* NewComponent(class ToyBase* _owner);
   PxScene* pPxScene;
};
}