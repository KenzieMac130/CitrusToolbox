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

#include "TestShape.hpp"
#include "im3d/im3d.h"

#include "middleware/PhysXIntegration.hpp"

ctHoneybell::TestShape::TestShape(ConstructContext& ctx) : ToyBase(ctx) {
   /* Setup Debug Shape */
   debugShape = ctx.pComponentRegistry->NewComponent<DebugShapeComponent>(this);
   ctBoundBox aabb = ctBoundBox();
   aabb.AddPoint(ctVec3(-0.5f));
   aabb.AddPoint(ctVec3(0.5f));
   debugShape->SetLocalBounds(aabb);

   const char* msg = ctx.spawn.message;
   char* msgnc = NULL;
   if (msg) { debugShape->rgba.r = strtof(msg, &msgnc); }
   msg = msgnc;
   if (msg) { debugShape->rgba.g = strtof(msg, &msgnc); }
   msg = msgnc;
   if (msg) { debugShape->rgba.b = strtof(msg, &msgnc); }
   msg = msgnc;
   if (msg) { debugShape->rgba.a = strtof(msg, &msgnc); }
   angle = 0.0f;

   /* Setup Physics */
   physxComp = ctx.pComponentRegistry->NewComponent<PhysXActorComponent>(this);
   physxComp->pPxRigidActor =
     ctx.pPhysics->createRigidDynamic(ctTransformToPx(GetWorldTransform()));
   physxComp->PxMaterialStorage.Append(ctx.pPhysics->createMaterial(0.5f, 0.5f, 0.6f));
   physxComp->PxShapeStorage.Append(ctx.pPhysics->createShape(
     PxBoxGeometry(0.5f, 0.5f, 0.5f), *physxComp->PxMaterialStorage[0]));
   physxComp->pPxRigidActor->attachShape(*physxComp->PxShapeStorage[0]);
}

ctHoneybell::TestShape::~TestShape() {
}

ctResults ctHoneybell::TestShape::OnBegin(BeginContext& ctx) {
   if (physxComp.isValid()) { physxComp->AddToScene(); }
   ctx.canFrameUpdate = false;
   ctx.canTickSerial = true;
   return CT_SUCCESS;
}

ctResults ctHoneybell::TestShape::OnTickSerial(TickContext& ctx) {
   /*ctTransform xform = GetWorldTransform();
   xform.position += xform.rotation.getForward() * (float)ctx.deltaTime * 3.0f;
   xform.rotation *= ctQuat(CT_VEC3_UP, angle * 0.005f);
   angle += (float)ctx.deltaTime;
   angle = ctSin(angle);
   if (physx.isValid()) {}
   SetWorldTransform(xform);*/
   if (physxComp.isValid()) { SetWorldTransform(physxComp->GetWorldTransform()); }
   if (debugShape.isValid()) { debugShape->CopyOwnerTransform(); }
   return CT_SUCCESS;
}

ctResults ctHoneybell::TestShape::OnFrameUpdate(FrameUpdateContext& ctx) {
#if CITRUS_IM3D
   Im3d::PushColor(Im3d::Color_Green);
   Im3d::DrawSphere(ctVec3ToIm3d(GetWorldTransform().position), 1.0f);
   Im3d::PopColor();
#endif
   return CT_SUCCESS;
}
