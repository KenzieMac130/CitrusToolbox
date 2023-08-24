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

enum ctPhysicsLayer {
   CT_PHYSICS_LAYER_STATIC = 0,             /* static bodies */
   CT_PHYSICS_LAYER_DYNAMIC_CORE = 1,       /* dynamic bodies which affect gameplay */
   CT_PHYSICS_LAYER_DYNAMIC_DETAIL = 2,     /* dynamic bodies for incidental effects */
   CT_PHYSICS_LAYER_DYNAMIC_RAGDOLL = 3,    /* ragdoll joints */
   CT_PHYSICS_LAYER_DYNAMIC_CONTROLLER = 4, /* character controllers */
   CT_PHYSICS_LAYER_COUNT,
   CT_PHYSICS_LAYER_MAX = 16,
   CT_PHYSICS_LAYER_DEFAULT = UINT16_MAX
};

class ctPhysicsCollisionMatrix {
public:
   inline ctPhysicsCollisionMatrix() {
      memset(data, 0, sizeof(data));

      /* static collides with all (predefined) but self */
      EnablePair(CT_PHYSICS_LAYER_STATIC, CT_PHYSICS_LAYER_DYNAMIC_CORE);
      EnablePair(CT_PHYSICS_LAYER_STATIC, CT_PHYSICS_LAYER_DYNAMIC_DETAIL);
      EnablePair(CT_PHYSICS_LAYER_STATIC, CT_PHYSICS_LAYER_DYNAMIC_RAGDOLL);
      EnablePair(CT_PHYSICS_LAYER_STATIC, CT_PHYSICS_LAYER_DYNAMIC_CONTROLLER);

      /* dynamic core collides with self and other dynamic */
      EnablePair(CT_PHYSICS_LAYER_DYNAMIC_CORE, CT_PHYSICS_LAYER_DYNAMIC_CORE);
      EnablePair(CT_PHYSICS_LAYER_DYNAMIC_CORE, CT_PHYSICS_LAYER_DYNAMIC_DETAIL);
      EnablePair(CT_PHYSICS_LAYER_DYNAMIC_CORE, CT_PHYSICS_LAYER_DYNAMIC_RAGDOLL);
      EnablePair(CT_PHYSICS_LAYER_DYNAMIC_CORE, CT_PHYSICS_LAYER_DYNAMIC_CONTROLLER);

      /* detail collides with self, dynamic core, and ragdolls */
      EnablePair(CT_PHYSICS_LAYER_DYNAMIC_DETAIL, CT_PHYSICS_LAYER_DYNAMIC_DETAIL);
      EnablePair(CT_PHYSICS_LAYER_DYNAMIC_DETAIL, CT_PHYSICS_LAYER_DYNAMIC_RAGDOLL);

      /* ragdolls collide with other ragdolls (by default) */
      EnablePair(CT_PHYSICS_LAYER_DYNAMIC_RAGDOLL, CT_PHYSICS_LAYER_DYNAMIC_RAGDOLL);

      /* controllers collide with other controllers and core/static */
      EnablePair(CT_PHYSICS_LAYER_DYNAMIC_CONTROLLER,
                 CT_PHYSICS_LAYER_DYNAMIC_CONTROLLER);
   }

   inline void EnablePair(ctPhysicsLayer a, ctPhysicsLayer b) {
      data[a] |= 1 << b;
      data[b] |= 1 << a;
   }

   inline void DisablePair(ctPhysicsLayer a, ctPhysicsLayer b) {
      data[a] &= ~(1 << b);
      data[b] &= ~(1 << a);
   }

   inline bool DoesCollide(ctPhysicsLayer a, ctPhysicsLayer b) {
      return ctCFlagCheck(a, b);
   }

private:
   uint16_t data[16];
};

enum ctPhysicsMotionType { CT_PHYSICS_STATIC, CT_PHYSICS_KINEMATIC, CT_PHYSICS_DYNAMIC };

enum ctPhysicsDOF {
   CT_PHYSICS_DOF_NONE = 0,
   CT_PHYSICS_DOF_TRANSLATION_X = 1,
   CT_PHYSICS_DOF_TRANSLATION_Y = 2,
   CT_PHYSICS_DOF_TRANSLATION_Z = 4,
   CT_PHYSICS_DOF_ROTATION_X = 8,
   CT_PHYSICS_DOF_ROTATION_Y = 16,
   CT_PHYSICS_DOF_ROTATION_Z = 32,
   CT_PHYSICS_DOF_TRANSLATION = CT_PHYSICS_DOF_TRANSLATION_X |
                                CT_PHYSICS_DOF_TRANSLATION_Y |
                                CT_PHYSICS_DOF_TRANSLATION_Z,
   CT_PHYSICS_DOF_ROTATION =
     CT_PHYSICS_DOF_ROTATION_X | CT_PHYSICS_DOF_ROTATION_Y | CT_PHYSICS_DOF_ROTATION_Z,
   CT_PHYSICS_DOF_ALL = CT_PHYSICS_DOF_TRANSLATION | CT_PHYSICS_DOF_ROTATION
};

struct ctPhysicsEngineDesc {
   size_t scratchAllocationSize = 10485760;
   size_t maxBodies = 65536;
   size_t maxBroadphasePairs = 65536;
   size_t maxContactConstraints = 10240;
   ctPhysicsCollisionMatrix collisionMatrix = ctPhysicsCollisionMatrix();
   const char* surfaceTypeJSON = NULL;
};

typedef struct ctPhysicsEngineT* ctPhysicsEngine;
ctResults ctPhysicsEngineStartup(ctPhysicsEngine& ctx, ctPhysicsEngineDesc& desc);
ctResults ctPhysicsEngineShutdown(ctPhysicsEngine ctx);
ctResults ctPhysicsEngineUpdate(ctPhysicsEngine ctx, float deltaTime, int32_t steps = 1);
ctResults ctPhysicsEngineExecDebugDraw(ctPhysicsEngine ctx);
ctResults ctPhysicsEngineExecDebugUI(ctPhysicsEngine ctx);

#include "Body.hpp"
#include "Character.hpp"
#include "Constraint.hpp"
#include "Query.hpp"
#include "Ragdoll.hpp"
#include "Shape.hpp"
#include "Module.hpp"