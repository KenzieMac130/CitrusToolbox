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
#include "JoltBaking.hpp"

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
   /* create the points */
   float convexRadius = 0.05f;
   JPH::Array<JPH::Vec3> points;
   points.resize(desc.convexHull.pointCount);
   for (uint32_t i = 0; i < desc.convexHull.pointCount; i++) {
      points[i] = ctVec3ToJolt(desc.convexHull.points[i]);
   }
   /* build convex hulls */
   if (desc.convexHull.ensureConvex) {
      /* setup the builder */
      JPH::ConvexHullBuilder builder = JPH::ConvexHullBuilder(points);
      const char* err = NULL;
      JPH::ConvexHullBuilder::EResult bres = builder.Initialize(INT32_MAX, 1.0e-3f, err);
      if (bres != JPH::ConvexHullBuilder::EResult::Success) {
         ctDebugError("Could not build convex hull: %s", err);
         return JPH::Shape::ShapeResult();
      }

      /* create a list of all used vertices */
      ctHashTable<bool, uint32_t> pointExists;
      pointExists.Reserve(builder.GetNumVerticesUsed());
      auto faces = builder.GetFaces();
      for (size_t f = 0; f < faces.size(); f++) {
         JPH::ConvexHullBuilder::Face* face = faces[f];
         JPH::ConvexHullBuilder::Edge* edge = face->mFirstEdge;
         for (size_t i = 0; i < 3; i++) {
            uint32_t idx = edge->mStartIdx;
            pointExists.InsertOrReplace(idx, true);
            edge = edge->mNextEdge;
         }
      }

      /* create point out*/
      JPH::Array<JPH::Vec3> pointsOut;
      pointsOut.reserve(pointExists.Count());
      for (auto it = pointExists.GetIterator(); it; it++) {
         pointsOut.push_back(points[it.Key()]);
      }
      points = pointsOut;
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
   if (desc.mesh.surfaceTypeHashCount) { /* multi material */
      physMats.resize(desc.mesh.surfaceTypeHashCount);
      for (uint32_t i = 0; i < desc.mesh.surfaceTypeHashCount; i++) {
         physMats[i] = ctx->GetMaterialForSurfaceType(desc.mesh.surfaceTypeHashes[i]);
      }
   } else { /* single material */
      physMats.resize(1);
      physMats[0] = ctx->GetMaterialForSurfaceType(desc.surfaceTypeHash);
   }

   JPH::MeshShapeSettings shape = JPH::MeshShapeSettings(verts, tris, physMats);
   return shape.Create();
}

JPH::Shape::ShapeResult CreateCompoundShape(const ctPhysicsEngine& ctx,
                                            ctPhysicsShapeSettings& desc) {
   JPH::StaticCompoundShapeSettings shape = JPH::StaticCompoundShapeSettings();
   JPH::Shape::ShapeResult subshapes[64];
   ctAssert(desc.compound.shapeCount <= 64);
   for (size_t i = 0; i < desc.compound.shapeCount; i++) {
      subshapes[i] =
        CreateShapeFromCitrus(ctx, desc.compound.shapes[i], CT_JOLT_SHAPE_LAYER_COMPOUND);
      shape.AddShape(ctVec3ToJolt(desc.compound.shapes[i].transform.translation),
                     ctQuatToJolt(desc.compound.shapes[i].transform.rotation),
                     subshapes[i].Get());
   }
   return shape.Create();
}

JPH::Shape::ShapeResult CreateBakedSubshape(const ctPhysicsEngine& ctx,
                                            CitrusJoltStreamIn stream) {
   /* load header and sections */
   CitrusJoltBakeHeader header;
   stream.ReadBytes(&header, sizeof(header));
   stream.Seek(header.sectionsOffset);
   CitrusJoltBakeSection sections[66];
   stream.ReadBytes(sections, sizeof(sections[0]) * header.sectionsCount);
   ctAssert(header.sectionsCount >= 2);

   /* load shape */
   stream.Seek((int64_t)sections[0].offset);
   JPH::Shape::ShapeResult result = JPH::Shape::sRestoreFromBinaryState(stream);

   /* load materials */
   stream.Seek((int64_t)sections[1].offset);
   JPH::PhysicsMaterialRefC materials[64];
   uint32_t* hashes = (uint32_t*)stream.CurrPtr();
   size_t hashCount = sections[1].size / sizeof(uint32_t);
   for (size_t i = 0; i < hashCount; i++) {
      materials[i] = ctx->GetMaterialForSurfaceType(hashes[i]);
   }
   result.Get()->RestoreMaterialState(materials, (unsigned int)hashCount);

   /* load subshapes */
   JPH::ShapeRefC subshapes[64];
   for (size_t i = 2; i < header.sectionsCount; i++) {
      stream.Seek((int64_t)sections[i].offset);
      subshapes[i - 2] = CreateBakedSubshape(ctx, stream).Get();
   }
   result.Get()->RestoreSubShapeState(subshapes, (unsigned int)header.sectionsCount - 2);
   return result;
}

JPH::Shape::ShapeResult CreateBakedShape(const ctPhysicsEngine& ctx,
                                         ctPhysicsShapeSettings& desc) {
   CitrusJoltStreamIn stream = CitrusJoltStreamIn(desc.baked.bytes, desc.baked.byteCount);
   return CreateBakedSubshape(ctx, stream);
}

ctVec3 MakeValidScale(ctVec3 scale, ctPhysicsShapeType type) {
   if (type == CT_PHYSICS_SHAPE_MESH || type == CT_PHYSICS_SHAPE_CONVEX_HULL) {
      return scale;
   } else {
      float maxValue = 0.05f;
      maxValue = ctMax(maxValue, scale.x);
      maxValue = ctMax(maxValue, scale.y);
      maxValue = ctMax(maxValue, scale.z);
      return ctVec3(maxValue);
   }
}

JPH::Shape::ShapeResult CreateShapeFromCitrus(const ctPhysicsEngine& ctx,
                                              ctPhysicsShapeSettings& desc,
                                              CreateShapeFromCitrusLayer layer) {
   if (layer == CT_JOLT_SHAPE_LAYER_CENTER_OF_MASS) {
      if (desc.centerOfMassOffset == ctVec3(0.0f)) {
         return CreateShapeFromCitrus(ctx, desc, CT_JOLT_SHAPE_LAYER_BASE);
      } else {
         return JPH::OffsetCenterOfMassShapeSettings(
                  ctVec3ToJolt(desc.centerOfMassOffset),
                  CreateShapeFromCitrus(ctx, desc, CT_JOLT_SHAPE_LAYER_BASE).Get())
           .Create();
      }
   } else if (layer == CT_JOLT_SHAPE_LAYER_SCALE) {
      if (desc.transform.scale == ctVec3(1.0f)) {
         return CreateShapeFromCitrus(ctx, desc, CT_JOLT_SHAPE_LAYER_CENTER_OF_MASS);
      } else {
         ctVec3 scale = MakeValidScale(desc.transform.scale, desc.type);
         return JPH::ScaledShapeSettings(
                  CreateShapeFromCitrus(ctx, desc, CT_JOLT_SHAPE_LAYER_CENTER_OF_MASS)
                    .Get(),
                  ctVec3ToJolt(scale))
           .Create();
      }
   } else if (layer == CT_JOLT_SHAPE_LAYER_TRANSFORM) {
      if (desc.transform.translation == ctVec3(0.0f) &&
          desc.transform.rotation == ctQuat()) {
         return CreateShapeFromCitrus(ctx, desc, CT_JOLT_SHAPE_LAYER_SCALE);

      } else {
         return JPH::RotatedTranslatedShapeSettings(
                  ctVec3ToJolt(desc.transform.translation),
                  ctQuatToJolt(desc.transform.rotation),
                  CreateShapeFromCitrus(ctx, desc, CT_JOLT_SHAPE_LAYER_SCALE).Get())
           .Create();
      }
   } else {
      switch (desc.type) {
         case CT_PHYSICS_SHAPE_BOX: return CreateBoxShape(ctx, desc);
         case CT_PHYSICS_SHAPE_SPHERE: return CreateSphereShape(ctx, desc);
         case CT_PHYSICS_SHAPE_CAPSULE: return CreateCapsuleShape(ctx, desc);
         case CT_PHYSICS_SHAPE_CONVEX_HULL: return CreateConvexHullShape(ctx, desc);
         case CT_PHYSICS_SHAPE_MESH: return CreateMeshShape(ctx, desc);
         case CT_PHYSICS_SHAPE_COMPOUND: return CreateCompoundShape(ctx, desc);
         case CT_PHYSICS_SHAPE_BAKED: return CreateBakedShape(ctx, desc);
         default: ctAssert(0); return JPH::Shape::ShapeResult();
      }
   }
}
