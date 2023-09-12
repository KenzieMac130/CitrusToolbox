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
#include "Shape.hpp"
#include "Physics.hpp"

/* for rigid bodies */

struct ctPhysicsBodyDesc {
   ctVec3 position = ctVec3();
   ctQuat rotation = ctQuat();
   ctVec3 initialLinearVelocity = ctVec3();
   ctVec3 initialAngularVelocity = ctVec3();
   ctPhysicsMotionType motion = CT_PHYSICS_DYNAMIC;
   ctPhysicsDOF allowedDOFs = CT_PHYSICS_DOF_ALL;
   ctPhysicsShapeSettings shape = ctPhysicsShapeBox();
   const char* bodyTag = "DEFAULT"; /* body tag name for gameplay purposes */
   bool isTrigger = false;          /* only fire callbacks */
   bool allowSleep = true;
   bool allowMoving = true;  /* a purely static object will be false */
   bool isDetail = false;    /* is detail physics object (shouldn't affect controller) */
   bool isHighSpeed = false; /* high speed objects can use continuous integration */
   bool startActive = false;
   float friction = 0.2f;
   float bounce = 0.0f;
   float linearDamp = 0.05f;
   float angularDamp = 0.05f;
   float maxLinearVelocity = 500.0f;
   float maxAngularVelocity = CT_PI * 0.25f * 60.0f;
   float gravityFactor = 1.0f;
   float massOverride = -1.0f;
   ctPhysicsLayer layerOverride = CT_PHYSICS_LAYER_DEFAULT;
};

typedef uint32_t ctPhysicsBody;
ctResults
ctPhysicsCreateBody(ctPhysicsEngine ctx, ctPhysicsBody& body, ctPhysicsBodyDesc& desc);
void ctPhysicsDestroy(ctPhysicsEngine ctx, ctPhysicsBody body);
bool ctPhysicsIsValid(ctPhysicsBody body);

/* todo: velocity and impulses */