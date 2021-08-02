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

#include "GroundPlane.hpp"

ctHoneybell::GroundPlane::GroundPlane(ConstructContext& ctx) : ToyBase(ctx) {
   PhysicsPlane = ctx.pComponentRegistry->NewComponent<PhysXActorComponent>(this);
   if (PhysicsPlane.isValid()) {
      PhysicsPlane->PxMaterialStorage.Append(
        ctx.pPhysics->createMaterial(0.5f, 0.5f, 0.6f));
      PhysicsPlane->pPxRigidActor =
        PxCreatePlane(*ctx.pPhysics,
                      PxPlane(CT_UP, -ctx.spawn.transform.position.CT_AXIS_VERTICAL),
                      *PhysicsPlane->PxMaterialStorage[0]);
   }
}

ctResults ctHoneybell::GroundPlane::OnBegin(BeginContext& ctx) {
   if (PhysicsPlane.isValid()) { PhysicsPlane->AddToScene(); }
   return CT_SUCCESS;
}
