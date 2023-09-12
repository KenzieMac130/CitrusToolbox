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
   /* get actual allowed movement */
   bool allowMove = desc.allowMoving;
   if (desc.motion != CT_PHYSICS_STATIC) { allowMove = true; }

   /* initialize any direct mapping */
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
   settings.mMotionQuality =
     desc.isHighSpeed ? JPH::EMotionQuality::LinearCast : JPH::EMotionQuality::Discrete;
   settings.mFriction = desc.friction;
   settings.mRestitution = desc.bounce;
   settings.mLinearDamping = desc.linearDamp;
   settings.mAngularDamping = desc.angularDamp;
   settings.mMaxLinearVelocity = desc.maxLinearVelocity;
   settings.mGravityFactor = desc.gravityFactor;
   settings.mUserData = ctXXHash32(desc.bodyTag);

   /* setup mass override */
   bool overrideMass = false;
   if (desc.massOverride > 0.0f) {
      overrideMass = true;
      JPH::MassProperties massProps = JPH::MassProperties();
      massProps.mMass = desc.massOverride;
      settings.mMassPropertiesOverride = massProps;
      settings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;
   }

   /* setup physics layer */
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

   /* setup shape */
   JPH::Shape::ShapeResult shape = CreateShapeFromCitrus(ctx, desc.shape);
   if (!shape.IsValid()) {
      ctDebugError("FAILED TO CREATE SHAPE FOR BODY AT (%f,%f,%f)!!!",
                   desc.position.x,
                   desc.position.y,
                   desc.position.z);
      settings.SetShape(new JPH::SphereShape(0.5f));
   } else {
      settings.SetShape(shape.Get());
   }

   /* fixup mass properties from shape  */
   if (allowMove && settings.GetMassProperties().mMass <= 0.0f && !overrideMass) {
      ctDebugWarning("MASS WAS LESS THAN ZERO FOR BODY AT (%f,%f,%f)!!!",
                     desc.position.x,
                     desc.position.y,
                     desc.position.z);
      JPH::MassProperties mass = JPH::MassProperties();
      mass.SetMassAndInertiaOfSolidBox(JPH::Vec3Arg(0.5f, 0.5f, 0.5f), 1.0f);
      settings.mOverrideMassProperties =
        JPH::EOverrideMassProperties::MassAndInertiaProvided;
      settings.mMassPropertiesOverride = mass;
   }

   /* create body */
   JPH::BodyInterface& binterface = ctx->physics.GetBodyInterface();
   body = binterface
            .CreateAndAddBody(settings,
                              desc.startActive ? JPH::EActivation::Activate
                                               : JPH::EActivation::DontActivate)
            .GetIndexAndSequenceNumber();
   return CT_SUCCESS;
}

void ctPhysicsDestroy(ctPhysicsEngine ctx, ctPhysicsBody body) {
   ctx->physics.GetBodyInterface().RemoveBody(JPH::BodyID(body));
   ctx->physics.GetBodyInterface().DestroyBody(JPH::BodyID(body));
}

bool ctPhysicsIsValid(ctPhysicsBody body) {
   return !((JPH::BodyID)body).IsInvalid();
}
