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
#include "JoltShape.hpp"
#include "Jolt/Geometry/ConvexHullBuilder.h"

JPH::Shape::ShapeResult CreateBoxShape(const ctPhysicsEngine& ctx,
                                       ctPhysicsShapeSettings& desc) {
   float convexRadius = 0.05f;
   if (convexRadius > desc.box.halfExtent.x) { convexRadius = desc.box.halfExtent.x; }
   if (convexRadius > desc.box.halfExtent.y) { convexRadius = desc.box.halfExtent.y; }
   if (convexRadius > desc.box.halfExtent.y) { convexRadius = desc.box.halfExtent.y; }
   JPH::BoxShapeSettings shape =
     JPH::BoxShapeSettings(ctVec3ToJolt(desc.box.halfExtent),
                           convexRadius,
                           ctx->GetMaterialForSurfaceType(desc.surfaceTypeHash));
   return shape.Create();
}

JPH::Shape::ShapeResult CreateSphereShape(const ctPhysicsEngine& ctx,
                                          ctPhysicsShapeSettings& desc) {
   JPH::SphereShapeSettings shape = JPH::SphereShapeSettings(
     desc.sphere.radius, ctx->GetMaterialForSurfaceType(desc.surfaceTypeHash));
   return shape.Create();
}

JPH::Shape::ShapeResult CreateCapsuleShape(const ctPhysicsEngine& ctx,
                                           ctPhysicsShapeSettings& desc) {
   JPH::CapsuleShapeSettings shape =
     JPH::CapsuleShapeSettings(desc.capsule.halfHeight,
                               desc.capsule.radius,
                               ctx->GetMaterialForSurfaceType(desc.surfaceTypeHash));
   return shape.Create();
}

JPH::Shape::ShapeResult CreateConvexHullShape(const ctPhysicsEngine& ctx,
                                              ctPhysicsShapeSettings& desc) {
   float convexRadius = 0.05f;
   JPH::Array<JPH::Vec3> points;
   points.resize(desc.convexHull.pointCount);
   for (uint32_t i = 0; i < desc.convexHull.pointCount; i++) {
      points[i] = ctVec3ToJolt(desc.convexHull.points[i]);
   }
   if (!desc.convexHull.skipBake) {
      JPH::ConvexHullBuilder builder = JPH::ConvexHullBuilder(points);
      const char* err = NULL;
      if (builder.Initialize(UINT32_MAX, 1.0e-3f, err) !=
          JPH::ConvexHullBuilder::EResult::Success) {
         ctDebugError("Could not build convex hull: %s", err);
         return JPH::Shape::ShapeResult();
      }
   }
   JPH::ConvexHullShapeSettings shape = JPH::ConvexHullShapeSettings(
     points, convexRadius, ctx->GetMaterialForSurfaceType(desc.surfaceTypeHash));
   return shape.Create();
}

JPH::Shape::ShapeResult CreateMeshShape(const ctPhysicsEngine& ctx,
                                        ctPhysicsShapeSettings& desc) {
   JPH::VertexList verts;
   JPH::IndexedTriangleList tris;
   verts.resize(desc.mesh.vertexCount);
   tris.resize(desc.mesh.indexCount / 3);
   for (uint32_t i = 0; i < desc.mesh.vertexCount; i++) {
      verts[i] = ctVec3ToJoltF3(desc.mesh.vertices[i]);
   }
   for (uint32_t i = 0; i < desc.mesh.indexCount / 3; i++) {
      uint32_t matIdx = 0;
      if (desc.mesh.triSurfaceIndexCount && desc.mesh.triSurfaceIndices) {
         matIdx = desc.mesh.triSurfaceIndices[i];
      }
      tris[i] = JPH::IndexedTriangle(desc.mesh.indices[i * 3 + 0],
                                     desc.mesh.indices[i * 3 + 1],
                                     desc.mesh.indices[i * 3 + 2],
                                     matIdx);
   }
   JPH::PhysicsMaterialList physMats = JPH::PhysicsMaterialList();
   physMats.resize(desc.mesh.surfaceTypeHashCount);
   for (uint32_t i = 0; i < desc.mesh.surfaceTypeHashCount; i++) {
      physMats[i] = ctx->GetMaterialForSurfaceType(desc.mesh.surfaceTypeHashes[i]);
   }

   JPH::MeshShapeSettings shape = JPH::MeshShapeSettings(verts, tris, physMats);
   return shape.Create();
}

JPH::Shape::ShapeResult CreateCompoundShape(const ctPhysicsEngine& ctx,
                                            ctPhysicsShapeSettings& desc) {
   JPH::StaticCompoundShapeSettings shape = JPH::StaticCompoundShapeSettings();
   JPH::Shape::ShapeResult subshapes[64];
   ctAssert(desc.compound.shapeCount < 64);
   for (size_t i = 0; i < desc.compound.shapeCount; i++) {
      subshapes[i] = CreateShapeFromCitrus(ctx, desc.compound.shapes[i], true);
      shape.AddShape(ctVec3ToJolt(desc.compound.shapes[i].transform.translation),
                     ctQuatToJolt(desc.compound.shapes[i].transform.rotation),
                     subshapes[i].Get());
      /* todo: move from concrete to settings bucket */
   }
   return shape.Create();
}

JPH::Shape::ShapeResult CreateShapeFromCitrus(const ctPhysicsEngine& ctx,
                                              ctPhysicsShapeSettings& desc,
                                              bool isNested) {
   /* todo: scale, rotate, translate, offset */
   switch (desc.type) {
      case CT_PHYSICS_SHAPE_BOX: return CreateBoxShape(ctx, desc);
      case CT_PHYSICS_SHAPE_SPHERE: return CreateSphereShape(ctx, desc);
      case CT_PHYSICS_SHAPE_CAPSULE: return CreateCapsuleShape(ctx, desc);
      case CT_PHYSICS_SHAPE_CONVEX_HULL: return CreateConvexHullShape(ctx, desc);
      case CT_PHYSICS_SHAPE_MESH: return CreateMeshShape(ctx, desc);
      case CT_PHYSICS_SHAPE_COMPOUND: return CreateCompoundShape(ctx, desc);
      default: ctAssert(0); return JPH::Shape::ShapeResult();
   }
}
