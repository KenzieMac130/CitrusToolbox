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
#include "Body.hpp"
#include "animation/Spline.hpp"

/* rigid body constraints */

enum ctPhysicsConstraintType {
   CT_PHYSICS_CONSTRAINT_NDOF,     /* configurable constraint */
   CT_PHYSICS_CONSTRAINT_HINGE,    /* for single axis rotation constraints */
   CT_PHYSICS_CONSTRAINT_SOCKET,   /* for shoulder-like rotation constraints */
   CT_PHYSICS_CONSTRAINT_POINT,    /* swings a child body around a parent point  */
   CT_PHYSICS_CONSTRAINT_FIXED,    /* forces a child body to maintain transforms */
   CT_PHYSICS_CONSTRAINT_GLUE,     /* fixed but uses the body world space at runtime */
   CT_PHYSICS_CONSTRAINT_DISTANCE, /* maintains a childs distance from a parent */
   CT_PHYSICS_CONSTRAINT_CONE,     /* similar to socket but simplified */
   CT_PHYSICS_CONSTRAINT_SPLINE,   /* gives a parent body a path the child can follow  */
   CT_PHYSICS_CONSTRAINT_GEAR,     /* gives a gear ratio rotation between parent/child */
   CT_PHYSICS_CONSTRAINT_RACK_PINION, /* the parent acts as a gear, the child is pinion */
   CT_PHYSICS_CONSTRAINT_PULLEY,      /* a pulley between two bodies */
   CT_PHYSICS_CONSTRAINT_BAKED        /* contraints which have been baked down */
};

struct ctPhysicsConstraintSpringSettings {
   ctPhysicsConstraintSpringSettings() {
      useFrequency = false;
      frequency = 0.0f;
      damping = 0.0f;
   }
   bool useFrequency; /* use frequency rather than stiffness */
   union {
      float frequency; /* oscilation frequency in Hz (0 for off) */
      float stiffness; /* stiffness as a direct variable in the spring equation */
   };
   float damping; /* damping from 0-1 or as component in spring equation */
};

enum ctPhysicsMotorState {
   CT_PHYSICS_MOTOR_OFF,
   CT_PHYSICS_MOTOR_VELOCITY,
   CT_PHYSICS_MOTOR_POSITIONAL
};

struct ctPhysicsConstraintMotorSettings {
   ctPhysicsConstraintMotorSettings() {
      positionalSpring.useFrequency = true;
      positionalSpring.frequency = 2.0f;
      positionalSpring.damping = 1.0f;
      minForceLimit = -FLT_MAX;
      maxForceLimit = FLT_MAX;
      minTorqueLimit = -FLT_MAX;
      maxTorqueLimit = FLT_MAX;
   }
   ctPhysicsConstraintSpringSettings positionalSpring;
   float minForceLimit;
   float maxForceLimit;
   float minTorqueLimit;
   float maxTorqueLimit;
};

enum ctPhysicsPathConstraintRotation {
   CT_PHYSICS_PATH_ROTATION_FREE,
   CT_PHYSICS_PATH_ROTATION_TANGENT,
   CT_PHYSICS_PATH_ROTATION_NORMAL,
   CT_PHYSICS_PATH_ROTATION_BINORMAL,
   CT_PHYSICS_PATH_ROTATION_COASTER,
   CT_PHYSICS_PATH_ROTATION_LOCK_PARENT
};

struct ctPhysicsConstraintSettings {
   ctPhysicsConstraintSettings() {
   }
   ctPhysicsConstraintType type;

   bool isLocalSpace;     /* specify parent/child transform in local or world space */
   ctVec3 parentPosition; /* parent reference frame for a constraint */
   ctQuat parentRotation;

   ctVec3 childPosition; /* child reference frame for a constraint */
   ctVec3 childRotation;

   union {
      struct {
         ctPhysicsDOF axis;

         ctVec3 translationalFriction;
         ctVec3 rotationalFriction;

         ctVec3 translationMin;
         ctVec3 translationMax;
         ctVec3 rotationEulerMin;
         ctVec3 rotationEulerMax;

         ctPhysicsConstraintSpringSettings translationSprings[3];

         ctPhysicsConstraintMotorSettings translationMotors[3];
         ctPhysicsConstraintMotorSettings rotationEulerMotors[3];
      } ndof;
      struct {
         float minAngle;
         float maxAngle;
         float friction;
         ctPhysicsConstraintSpringSettings spring;
         ctPhysicsConstraintMotorSettings motor;
      } hinge;
      struct {
         ctAxis parentTwistAxis;
         ctAxis parentPlaneAxis;
         ctAxis childTwistAxis;
         ctAxis childPlaneAxis;

         float normalHalfConeAngle;
         float planeHalfAngle;

         float twistMinAngle;
         float twistMaxAngle;

         float friction;

         ctPhysicsConstraintMotorSettings swingMotor;
         ctPhysicsConstraintMotorSettings twistMotor;
      } socket;
      struct {
         float minDistance;
         float maxDistance;
         ctPhysicsConstraintSpringSettings spring;
      } distance;
      struct {
         ctAxis parentTwistAxis;
         ctAxis childTwistAxis;
         float coneAngle;
      } cone;
      struct {
         ctAnimSpline* pSpline;
         bool calculateAttachFactor; /* calculate attach factor by nearest point */
         float attachFactor;         /* factor at which the "rod attaches to the rail" */
         float friction;
         ctPhysicsPathConstraintRotation rotation;
         ctPhysicsConstraintMotorSettings positionMotor;
      } spline;
      struct {
         ctAxis parentHingeAxis;
         ctAxis childHigeAxis;
         float ratio;
      } gear;
      struct {
         ctAxis pinionHingeAxis;
         ctAxis rackSliderAxis;
         float ratio;
      } rackAndPinion;
      struct {
         ctVec3 parentHookWorldPosition;
         ctVec3 childHookWorldPosition;
         float ratio;     /* ratio in length between segments */
         float minLength; /* 0 is good for rope/chain, -1 automatically calculates */
         float maxLength; /* -1 automatically calculates */
      } pulley;
      struct {
         uint8_t* bytes;
         size_t byteCount;
      } baked;
   };
};

typedef void* ctPhysicsConstraint;
ctResults ctPhysicsCreateConstraint(ctPhysicsEngine ctx,
                                    ctPhysicsConstraint& constraint,
                                    ctPhysicsConstraintSettings& settings,
                                    ctPhysicsBody child,
                                    ctPhysicsBody parent = UINT32_MAX);
void ctPhysicsDestroy(ctPhysicsEngine ctx, ctPhysicsConstraint constraint);
bool ctPhysicsIsValid(ctPhysicsConstraint constraint);