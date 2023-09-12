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
#include "Constraint.hpp"
#include "animation/Skeleton.hpp"

/* ------------------------- Shape ------------------------- */

/* bakes a shape into a binary format to be loaded from later */
ctResults ctPhysicsBakeShape(ctPhysicsEngine ctx,
                             ctPhysicsShapeSettings& shape,
                             ctDynamicArray<uint8_t>& output);

/* ------------------------- Constraint ------------------------- */

/* bakes a constraint description into a binary format to be loaded from later */
ctResults ctPhysicsBakeConstraint(ctPhysicsEngine ctx,
                                  ctPhysicsConstraintSettings& constraint,
                                  ctDynamicArray<uint8_t>& output);

/* ------------------------- Mesh Extraction ------------------------- */

/* gets a unindexed mesh representation of a shape, useful for navmesh baking */
ctResults ctPhysicsShapeToMesh(ctPhysicsEngine ctx,
                               ctPhysicsShapeSettings& shape,
                               ctDynamicArray<ctVec3>& vertices);

/* ------------------------- Convex Decompose ------------------------- */
enum ctPhysicsConvexDecompositionFill {
   CT_PHYSICS_CONVEX_DECOMP_FLOOD_FILL,
   CT_PHYSICS_CONVEX_DECOMP_SURFACE_ONLY,
   CT_PHYSICS_CONVEX_DECOMP_RAYCAST_FILL
};

/* uses vhacd for convex hull decomposition, use in conjuntion with ctPhysicsBakeShape */
class ctPhysicsConvexDecomposition {
public:
   ctPhysicsConvexDecomposition() = default;
   ctPhysicsConvexDecomposition(const ctPhysicsConvexDecomposition& other) = delete;
   ~ctPhysicsConvexDecomposition() = default;

   uint32_t maxHulls = 64;
   uint32_t resolution = 400000;
   float minVolumePercentError = 1.0f;
   uint32_t maxRecursion = 10;
   bool shrinkWrap = true;
   ctPhysicsConvexDecompositionFill fill = CT_PHYSICS_CONVEX_DECOMP_FLOOD_FILL;
   uint32_t maxVertsPerHull = 64;
   uint32_t minEdgeLength = 2;

   ctResults ProcessMesh(ctVec3* pVertices,
                         size_t vertexCount,
                         uint32_t* pIndices,
                         size_t indexCount,
                         uint32_t surfaceTypeHash);

   inline ctPhysicsShapeSettings& GetShape() {
      return rootShape;
   }

private:
   ctPhysicsShapeSettings rootShape;
   ctDynamicArray<ctPhysicsShapeSettings> subshapes;
   ctDynamicArray<ctVec3> pointsAll;
};

/* ------------------------- Ragdoll ------------------------- */

struct ctPhysicsBakeRagdollBone {
   ctPhysicsShapeSettings shape;
   ctPhysicsConstraintSettings toParent;
};

struct ctPhysicsBakeRagdollDesc {
   const ctAnimSkeleton* pAnimationSkeleton;
   const ctAnimSkeleton* pRagdollSkeleton;
   ctPhysicsBakeRagdollBone* pRagdollBones; /* must match ragdoll skeleton bone count */
};

ctResults ctPhysicsBakeRagdoll(ctPhysicsEngine ctx,
                               ctPhysicsBakeRagdollDesc& desc,
                               ctDynamicArray<uint8_t>& output);