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
   CT_PHYSICS_SHAPE_COMPOUND,
   CT_PHYSICS_SHAPE_BAKED
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
         bool ensureConvex;
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
      struct {
         uint8_t* bytes;
         size_t byteCount;
      } baked;
   };
};

inline ctPhysicsShapeSettings ctPhysicsShapeBox(ctVec3 halfExtent = ctVec3(0.5f),
                                                uint32_t surfaceTypeHash = 0,
                                                ctTransform transform = ctTransform(),
                                                ctVec3 centerOfMassOffset = ctVec3()) {
   if (halfExtent.x < 0.005f) { halfExtent.x = 0.005f; }
   if (halfExtent.y < 0.005f) { halfExtent.y = 0.005f; }
   if (halfExtent.z < 0.005f) { halfExtent.z = 0.005f; }
   ctPhysicsShapeSettings result = {};
   result.type = CT_PHYSICS_SHAPE_BOX;
   result.transform = transform;
   result.centerOfMassOffset = centerOfMassOffset;
   result.surfaceTypeHash = surfaceTypeHash;
   result.box.halfExtent = halfExtent;
   return result;
}

inline ctPhysicsShapeSettings ctPhysicsShapeSphere(float radius = 0.5f,
                                                   uint32_t surfaceTypeHash = 0,
                                                   ctTransform transform = ctTransform(),
                                                   ctVec3 centerOfMassOffset = ctVec3()) {
   if (radius < 0.005f) { radius = 0.005f; }
   ctPhysicsShapeSettings result = {};
   result.type = CT_PHYSICS_SHAPE_SPHERE;
   result.transform = transform;
   result.centerOfMassOffset = centerOfMassOffset;
   result.surfaceTypeHash = surfaceTypeHash;
   result.sphere.radius = radius;
   return result;
}

inline ctPhysicsShapeSettings
ctPhysicsShapeCapsule(float radius = 0.5f,
                      float halfHeight = 0.5f,
                      uint32_t surfaceTypeHash = 0,
                      ctTransform transform = ctTransform(),
                      ctVec3 centerOfMassOffset = ctVec3()) {
   if (radius < 0.005f) { radius = 0.005f; }
   if (halfHeight < 0.0f) { halfHeight = 0.0f; }
   ctPhysicsShapeSettings result = {};
   result.type = CT_PHYSICS_SHAPE_CAPSULE;
   result.transform = transform;
   result.centerOfMassOffset = centerOfMassOffset;
   result.surfaceTypeHash = surfaceTypeHash;
   result.capsule.radius = radius;
   result.capsule.halfHeight = halfHeight;
   return result;
}

inline ctPhysicsShapeSettings
ctPhysicsShapeConvexHull(ctVec3* points,
                         uint32_t pointCount,
                         uint32_t surfaceTypeHash = 0,
                         ctTransform transform = ctTransform(),
                         ctVec3 centerOfMassOffset = ctVec3()) {
   ctPhysicsShapeSettings result = {};
   result.type = CT_PHYSICS_SHAPE_CONVEX_HULL;
   result.transform = transform;
   result.centerOfMassOffset = centerOfMassOffset;
   result.surfaceTypeHash = surfaceTypeHash;
   result.convexHull.pointCount = pointCount;
   result.convexHull.points = points;
   result.convexHull.ensureConvex = true;
   return result;
}

inline ctPhysicsShapeSettings ctPhysicsShapeMesh(ctVec3* vertices,
                                                 uint32_t vertexCount,
                                                 uint32_t* indices,
                                                 uint32_t indexCount,
                                                 uint32_t singleSurfaceTypeHash = 0,
                                                 uint32_t* triSurfaceIndices = NULL,
                                                 uint32_t triSurfaceIndexCount = 0,
                                                 uint32_t* surfaceTypeHashes = NULL,
                                                 uint32_t surfaceTypeHashCount = 0,
                                                 ctTransform transform = ctTransform(),
                                                 ctVec3 centerOfMassOffset = ctVec3()) {
   ctPhysicsShapeSettings result = {};
   result.type = CT_PHYSICS_SHAPE_MESH;
   result.transform = transform;
   result.centerOfMassOffset = centerOfMassOffset;
   result.surfaceTypeHash = singleSurfaceTypeHash;
   result.mesh.vertices = vertices;
   result.mesh.vertexCount = vertexCount;
   result.mesh.indices = indices;
   result.mesh.indexCount = indexCount;
   result.mesh.triSurfaceIndices = triSurfaceIndices;
   result.mesh.triSurfaceIndexCount = triSurfaceIndexCount;
   result.mesh.surfaceTypeHashes = surfaceTypeHashes;
   result.mesh.surfaceTypeHashCount = surfaceTypeHashCount;
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

inline ctPhysicsShapeSettings ctPhysicsShapeBaked(uint8_t* bytes, size_t byteCount) {
   ctPhysicsShapeSettings result = {};
   result.type = CT_PHYSICS_SHAPE_BAKED;
   result.transform = ctTransform();
   result.centerOfMassOffset = ctVec3();
   result.baked.bytes = bytes;
   result.baked.byteCount = byteCount;
   return result;
}