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

enum ctPhysicsShapeType {
   CT_PHYSICS_SHAPE_BOX,
   CT_PHYSICS_SHAPE_SPHERE,
   CT_PHYSICS_SHAPE_CAPSULE,
   CT_PHYSICS_SHAPE_CONVEX_HULL,
   CT_PHYSICS_SHAPE_MESH,
   CT_PHYSICS_SHAPE_COMPOUND
};

struct ctPhysicsShapeSettings {
   ctPhysicsShapeSettings() {
   }
   ctPhysicsShapeType type;
   ctTransform transform;
   ctVec3 centerOfMassOffset;
   uint32_t surfaceTypeHash;
   union {
      struct {
         ctVec3 halfExtent;
      } box;
      struct {
         float radius;
      } sphere;
      struct {
         float radius;
         float halfHeight;
      } capsule;
      struct {
         ctVec3* points;
         uint32_t pointCount;
         bool skipBake;
      } convexHull;
      struct {
         ctVec3* vertices;
         size_t vertexCount;

         uint32_t* indices;
         size_t indexCount;

         uint32_t* triSurfaceIndices;
         size_t triSurfaceIndexCount;

         uint32_t* surfaceTypeHashes;
         size_t surfaceTypeHashCount;
      } mesh;
      struct {
         ctPhysicsShapeSettings* shapes;
         size_t shapeCount;
      } compound;
   };
};

inline ctPhysicsShapeSettings ctPhysicsShapeBox(ctVec3 halfExtent = ctVec3(0.5f),
                                                ctTransform transform = ctTransform(),
                                                ctVec3 centerOfMassOffset = ctVec3()) {
   ctPhysicsShapeSettings result = {};
   result.type = CT_PHYSICS_SHAPE_BOX;
   result.centerOfMassOffset = centerOfMassOffset;
   result.box.halfExtent = halfExtent;
   return result;
}

inline ctPhysicsShapeSettings ctPhysicsShapeSphere(float radius = 0.5f,
                                                   ctTransform transform = ctTransform(),
                                                   ctVec3 centerOfMassOffset = ctVec3()) {
   ctPhysicsShapeSettings result = {};
   result.type = CT_PHYSICS_SHAPE_SPHERE;
   result.centerOfMassOffset = centerOfMassOffset;
   result.sphere.radius = radius;
   return result;
}

inline ctPhysicsShapeSettings
ctPhysicsShapeCapsule(float radius = 0.5f,
                      float halfHeight = 0.5f,
                      ctTransform transform = ctTransform(),
                      ctVec3 centerOfMassOffset = ctVec3()) {
   ctPhysicsShapeSettings result = {};
   result.type = CT_PHYSICS_SHAPE_CAPSULE;
   result.transform = transform;
   result.centerOfMassOffset = centerOfMassOffset;
   result.capsule.radius = radius;
   result.capsule.halfHeight = halfHeight;
   return result;
}

inline ctPhysicsShapeSettings
ctPhysicsShapeConvexHull(ctVec3* points,
                         uint32_t pointCount,
                         ctTransform transform = ctTransform(),
                         ctVec3 centerOfMassOffset = ctVec3()) {
   ctPhysicsShapeSettings result = {};
   result.type = CT_PHYSICS_SHAPE_CONVEX_HULL;
   result.transform = transform;
   result.centerOfMassOffset = centerOfMassOffset;
   result.convexHull.pointCount = pointCount;
   result.convexHull.points = points;
   return result;
}

inline ctPhysicsShapeSettings ctPhysicsShapeMesh(ctVec3* vertices,
                                                 uint32_t vertexCount,
                                                 uint32_t* indices,
                                                 uint32_t indexCount,
                                                 ctTransform transform = ctTransform(),
                                                 ctVec3 centerOfMassOffset = ctVec3()) {
   ctPhysicsShapeSettings result = {};
   result.type = CT_PHYSICS_SHAPE_CONVEX_HULL;
   result.transform = transform;
   result.centerOfMassOffset = centerOfMassOffset;
   result.mesh.vertices = vertices;
   result.mesh.indices = indices;
   result.mesh.vertexCount = vertexCount;
   result.mesh.indexCount = indexCount;
   return result;
}

inline ctPhysicsShapeSettings ctPhysicsShapeCompound(size_t count,
                                                     ctPhysicsShapeSettings* others) {
   ctPhysicsShapeSettings result = {};
   result.type = CT_PHYSICS_SHAPE_COMPOUND;
   result.transform = ctTransform();
   result.centerOfMassOffset = ctVec3();
   result.compound.shapeCount = count;
   result.compound.shapes = others;
   return result;
}