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
#include "formats/model/Model.hpp"
#include "Physics.hpp"
#include "Body.hpp"
#include "animation/Skeleton.hpp"

/* ----------------- Ragdoll Factory ----------------- */
struct ctPhysicsRagdollFactoryDesc {
   size_t bakeSize;
   uint8_t* bakeData;
};

typedef class ctPhysicsRagdollFactoryT* ctPhysicsRagdollFactory;
/* Notice: the ownership of ragdoll factories are shared among ragdolls.
Memory for them will be cleaned up once this reference and every other
radoll created by it is destroyed. Access is locked and threadsafe.
The ragdoll factory is meant to be used by the resource manager. */
ctResults ctPhysicsCreateRagdollFactory(ctPhysicsRagdollFactory& factory,
                                        ctPhysicsRagdollFactoryDesc& desc);
void ctPhysicsReleaseRagdollFactory(ctPhysicsRagdollFactory& factory);

/* ----------------- Ragdoll Instance ----------------- */
struct ctPhysicsRagdollDesc {
   float friction = 0.2f;
   float bounce = 0.0f;
   float linearDamp = 0.05f;
   float angularDamp = 0.05f;
   float maxLinearVelocity = 500.0f;
   float maxAngularVelocity = CT_PI * 0.25f * 60.0f;
   float gravityFactor = 1.0f;
   float massOverride = -1.0f;
   bool isHighSpeed = false;
   bool startActive = false;
   ctPhysicsLayer layerOverride = CT_PHYSICS_LAYER_DEFAULT;
   const char* bodyTag = "DEFAULT"; /* body tag name for gameplay purposes */
};

typedef class ctPhysicsRagdollT* ctPhysicsRagdoll;
ctResults ctPhysicsCreateRagdoll(ctPhysicsEngine ctx,
                                 ctPhysicsRagdollFactory& ragdoll,
                                 ctPhysicsRagdollDesc& desc);
void ctPhysicsDestroy(ctPhysicsEngine ctx, ctPhysicsRagdoll& ragdoll);

ctResults ctPhysicsRagdollToSkeleton(const ctPhysicsRagdoll& ragdoll,
                                     ctVec3 worldTranslation,
                                     ctVec3 worldRotation,
                                     ctAnimSkeleton& skeleton);

ctResults ctPhysicsRagdollDriveKinematic(ctPhysicsRagdoll& ragdoll,
                                         ctVec3 worldTranslation,
                                         ctVec3 worldRotation,
                                         const ctAnimSkeleton& skeleton,
                                         float deltaTime);
ctResults ctPhysicsRagdollDriveSkeletonMotor(ctPhysicsRagdoll& ragdoll,
                                             ctVec3 worldTranslation,
                                             ctVec3 worldRotation,
                                             const ctAnimSkeleton& skeleton);

/* use ctPhysicsBodyIsValid() to check if bone returned a body
DO NOT DESTROY RETURNED BODY! */
ctPhysicsBody ctPhysicsRagdollGetBodyForBone(ctAnimBone bone);

/* use ctPhysicsConstraintIsValid() to check if bone returned a constraint
DO NOT DESTROY RETURNED CONSTRAINT! */
// ctPhysicsConstraint ctPhysicsRagdollGetParentConstraintForBone(ctAnimBone bone);

/* todo: velocity and impulses */