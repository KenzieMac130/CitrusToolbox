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
#include "JoltBody.hpp"

/* for rigid bodies */

ctResults
ctPhysicsCreateBody(ctPhysicsEngine ctx, ctPhysicsBody& body, ctPhysicsBodyDesc& desc) {
   bool allowMove = desc.allowMoving;
   if (desc.motion != CT_PHYSICS_STATIC) { allowMove = true; }
   JPH::BodyCreationSettings settings = JPH::BodyCreationSettings();
   settings.mPosition = ctVec3ToJolt(desc.position);
   settings.mRotation = ctQuatToJolt(desc.rotation);
   settings.mLinearVelocity = ctVec3ToJolt(desc.initialLinearVelocity);
   settings.mAngularVelocity = ctVec3ToJolt(desc.initialAngularVelocity);
   settings.mMotionType = ctPhysicsMotionTypeToJolt(desc.motion);
   settings.mAllowedDOFs = ctPhysicsDOFToJolt(desc.allowedDOFs);
   settings.mIsSensor = desc.isTrigger;
   settings.mAllowSleeping = desc.allowSleep;
   settings.mAllowDynamicOrKinematic = allowMove;
   if (desc.layerOverride != CT_PHYSICS_LAYER_DEFAULT) {
      settings.mObjectLayer = desc.layerOverride;
   } else { /* auto select layer */
      if (allowMove) {
         settings.mObjectLayer = desc.isDetail ? CT_PHYSICS_LAYER_DYNAMIC_DETAIL
                                               : CT_PHYSICS_LAYER_DYNAMIC_CORE;
      } else {
         settings.mObjectLayer = CT_PHYSICS_LAYER_STATIC;
      }
   }
   settings.mMotionQuality =
     desc.isHighSpeed ? JPH::EMotionQuality::LinearCast : JPH::EMotionQuality::Discrete;
   settings.mFriction = desc.friction;
   settings.mRestitution = desc.bounce;
   settings.mLinearDamping = desc.linearDamp;
   settings.mAngularDamping = desc.angularDamp;
   settings.mMaxLinearVelocity = desc.maxLinearVelocity;
   settings.mGravityFactor = desc.gravityFactor;
   settings.mUserData = ctXXHash32(desc.bodyTag);

   JPH::Shape::ShapeResult shape = CreateShapeFromCitrus(ctx, desc.shape);
   if (!shape.IsValid()) {
      ctDebugError("FAILED TO CREATE SHAPE FOR BODY AT (%f,%f,%f)!!!");
      settings.SetShape(new JPH::SphereShape(0.5f));
   } else {
      settings.SetShape(shape.Get());
   }

   JPH::BodyInterface& interfa = ctx->physics.GetBodyInterface();
   body = interfa
            .CreateAndAddBody(settings,
                              desc.startActive ? JPH::EActivation::Activate
                                               : JPH::EActivation::DontActivate)
            .GetIndexAndSequenceNumber();
   return CT_SUCCESS;
}

void ctPhysicsDestroy(ctPhysicsEngine ctx, ctPhysicsBody body) {
   ctx->physics.GetBodyInterface().RemoveBody(JPH::BodyID(body));
}
