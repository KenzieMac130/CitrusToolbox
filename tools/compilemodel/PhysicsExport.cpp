/*
   Copyright 2022 MacKenzie Strand

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

#include "../engine/utilities/Common.h"
#include "CitrusModel.hpp"

ctTransform ctGltf2Model::GetCollisionTransform(const cgltf_node& node,
                                                bool absolute,
                                                ctVec3 translateLocal,
                                                bool isMesh) {
   translateLocal = translateLocal * ctQuat(node.rotation);
   /* nodes without meshes just indicate to use the parent as an input */
   if (isMesh && !node.mesh) {
      if (!node.parent) { return ctTransform(); /* shouldn't happen! */ }
      return GetCollisionTransform(*node.parent, absolute, translateLocal);
   }

   /* return the transform of the offset and the collision subnode from parent */
   if (!absolute || !node.parent) {
      return ctTransform(
        ctVec3(node.translation) + translateLocal, node.rotation, node.scale);
   }

   /* Return final world transformation */
   ctMat4 parentMatrix = WorldMatrixFromGltfNodeIdx((uint32_t)(node.parent - gltf.nodes));
   ctTransform localTransform =
     ctTransform(ctVec3(node.translation) + translateLocal, node.rotation, node.scale);
   ctMat4 localMatrix;
   ctMat4FromTransform(localMatrix, localTransform);
   ctMat4 finalMatrix = parentMatrix * localMatrix;
   ctTransform finalTransform;
   ctMat4AwkwardDecompose(finalMatrix, finalTransform);
   return finalTransform;
}

void ctGltf2Model::GetCollisionDimensions(const cgltf_node& node,
                                          ctVec3& localTranslation,
                                          ctVec3& comOffset,
                                          ctVec3& halfExtents) {
   if (node.mesh) {
      ctBoundBox bbox = ctBoundBox(); /* calculate bbox */
      for (size_t p = 0; p < node.mesh->primitives_count; p++) {
         cgltf_primitive& prim = node.mesh->primitives[p];
         for (size_t a = 0; a < prim.attributes_count; a++) {
            cgltf_attribute& attrib = prim.attributes[a];
            if (attrib.type == cgltf_attribute_type_position) {
               ctDynamicArray<ctVec3> positions;
               ctAssert(attrib.data);
               positions.Resize(attrib.data->count);
               CopyAccessorToReserve(*attrib.data,
                                     (uint8_t*)positions.Data(),
                                     TinyImageFormat_R32G32B32_SFLOAT,
                                     sizeof(ctVec3),
                                     0);
               for (size_t i = 0; i < positions.Count(); i++) {
                  bbox.AddPoint(positions[i]);
               }
               break;
            }
         }
      }
      /* translation is the center of the bounding box plus the local translation */
      localTranslation = bbox.Center();
      if (ctMultiFloatCompare(3, localTranslation.data, ctVec3(0.0f).data, 0.05f)) {
         localTranslation = ctVec3(0.0f);
      }
      halfExtents = (bbox.max - bbox.min) / 2.0f;
   } else { /* otherwise its just half the node scale */
      halfExtents = ctVec3(0.5f);
      localTranslation = ctVec3(0.0f);
   }
   /* center of mass offset is the offset from bbox translation */
   comOffset = -localTranslation;
   if (ctMultiFloatCompare(3, comOffset.data, ctVec3(0.0f).data, 0.05f)) {
      comOffset = ctVec3(0.0f);
   }
}

uint32_t ctGltf2Model::GetSurfaceHashForPrimitive(const cgltf_node& node,
                                                  uint32_t primIdx) {
   if (!node.mesh) { return 0; }
   return GetSurfaceHashForPrimitive(*node.mesh, primIdx);
}

uint32_t ctGltf2Model::GetSurfaceHashForPrimitive(const cgltf_mesh& mesh,
                                                  uint32_t primIdx) {
   if (mesh.primitives_count <= primIdx) { return 0; }
   if (!mesh.primitives[primIdx].material) { return 0; }
   char tmpBuffer[33];
   memset(tmpBuffer, 0, 33);
   strncpy(tmpBuffer, mesh.primitives[primIdx].material->name, 32);
   if (!ctCStrNEql(tmpBuffer, "CM_", 3)) { return 0; }
   return ctXXHash32(tmpBuffer + 3, 32 - 3);
}

cgltf_mesh* ctGltf2Model::GetCollisionMeshForNode(const cgltf_node& node) {
   if (node.mesh) { return node.mesh; }
   if (node.parent) {
      if (node.parent->mesh) { return node.parent->mesh; }
   }
   return NULL;
}

void ctGltf2Model::GetCollisionMeshData(cgltf_mesh* input,
                                        ctGltf2ModelCollision* output,
                                        bool getIndices,
                                        bool getMaterialMap,
                                        ctVec3 scale) {
   for (size_t p = 0; p < input->primitives_count; p++) {
      cgltf_primitive& prim = input->primitives[p];
      /* load vertices*/
      for (size_t a = 0; a < prim.attributes_count; a++) {
         cgltf_attribute& attrib = prim.attributes[a];
         if (attrib.type == cgltf_attribute_type_position) {
            size_t posOffset = output->points.Count();
            ctAssert(attrib.data);
            output->points.Resize(output->points.Count() + attrib.data->count);
            CopyAccessorToReserve(*attrib.data,
                                  (uint8_t*)&output->points[posOffset],
                                  TinyImageFormat_R32G32B32_SFLOAT,
                                  sizeof(ctVec3),
                                  0);
         }
      }
      /* load indices */
      if (getIndices) {
         size_t idxOffset = output->indices.Count();
         ctAssert(prim.indices);
         output->indices.Resize(output->indices.Count() + prim.indices->count);
         CopyAccessorToReserve(*prim.indices,
                               (uint8_t*)&output->indices[idxOffset],
                               TinyImageFormat_R32_UINT,
                               sizeof(uint32_t),
                               0);
      }

      if (getMaterialMap) {
         /* append material slot */
         uint32_t matIdx = (uint32_t)output->materialHashes.Count();
         output->materialHashes.Append(GetSurfaceHashForPrimitive(*input, (uint32_t)p));

         /* create triangle material indices */
         output->materialIndices.Append(matIdx, prim.indices->count / 3);
      }
   }

   /* apply scale */
   for (size_t i = 0; i < output->points.Count(); i++) {
      output->points[i] = output->points[i] * scale;
   }
}

void ctGltf2Model::GetCollisionMeshDataFromVisuals(ctGltf2ModelCollision* output,
                                                   bool getIndices) {
   /* todo: load visual mesh into arrays */
   // for(size_t i = 0; i < tree.instances.Count(); i++)
}

ctGltf2ModelCollision* ctGltf2Model::GetBoxCollision(const cgltf_node& node,
                                                     bool absoluteTransform) {
   ctGltf2ModelCollision* result = new ctGltf2ModelCollision();
   ctVec3 halfExtents = ctVec3(0.5f);
   ctVec3 centerOfMass = ctVec3();
   ctVec3 translateOffset = ctVec3(0.0f);
   GetCollisionDimensions(node, translateOffset, centerOfMass, halfExtents);
   ctTransform xform = GetCollisionTransform(node, absoluteTransform, translateOffset);
   ctTransform xformNoscale = xform;
   xformNoscale.scale = ctVec3(1.0f);
   uint32_t surface = GetSurfaceHashForPrimitive(node);
   result->shapeSettings =
     ctPhysicsShapeBox(halfExtents * xform.scale, surface, xformNoscale, centerOfMass);
   return result;
}

ctGltf2ModelCollision* ctGltf2Model::GetSphereCollision(const cgltf_node& node,
                                                        bool absoluteTransform) {
   ctGltf2ModelCollision* result = new ctGltf2ModelCollision();
   ctVec3 halfExtents = ctVec3(0.5f);
   ctVec3 centerOfMass = ctVec3();
   ctVec3 translateOffset = ctVec3(0.0f);
   GetCollisionDimensions(node, translateOffset, centerOfMass, halfExtents);
   ctTransform xform = GetCollisionTransform(node, absoluteTransform, translateOffset);
   ctTransform xformNoscale = xform;
   xformNoscale.scale = ctVec3(1.0f);
   uint32_t surface = GetSurfaceHashForPrimitive(node);
   const float radius =
     ctMax(halfExtents.x * xform.scale.x,
           ctMax(halfExtents.y * xform.scale.y, halfExtents.z * xform.scale.z));
   result->shapeSettings =
     ctPhysicsShapeSphere(radius, surface, xformNoscale, centerOfMass);
   return result;
}

ctGltf2ModelCollision* ctGltf2Model::GetCapsuleCollision(const cgltf_node& node,
                                                         bool absoluteTransform) {
   ctGltf2ModelCollision* result = new ctGltf2ModelCollision();
   ctVec3 halfExtents = ctVec3(0.5f);
   ctVec3 centerOfMass = ctVec3();
   ctVec3 translateOffset = ctVec3(0.0f);
   GetCollisionDimensions(node, translateOffset, centerOfMass, halfExtents);
   ctTransform xform = GetCollisionTransform(node, absoluteTransform, translateOffset);
   ctTransform xformNoscale = xform;
   xformNoscale.scale = ctVec3(1.0f);
   uint32_t surface = GetSurfaceHashForPrimitive(node);
   result->shapeSettings =
     ctPhysicsShapeCapsule(halfExtents.x * xform.scale.x,
                           halfExtents.y * xform.scale.y - halfExtents.x * xform.scale.x,
                           surface,
                           xformNoscale,
                           centerOfMass);
   return result;
}

ctGltf2ModelCollision* ctGltf2Model::GetConvexCollision(const cgltf_node& node,
                                                        bool absoluteTransform) {
   cgltf_mesh* mesh = GetCollisionMeshForNode(node);
   if (!mesh) {
      ctDebugError("COLLISION NODE %s HAS NO MESH!", node.name);
      return NULL;
   }
   ctGltf2ModelCollision* result = new ctGltf2ModelCollision();
   ctVec3 halfExtents = ctVec3(0.5f);
   ctVec3 centerOfMass = ctVec3();
   ctTransform xform = GetCollisionTransform(node, absoluteTransform, ctVec3(0.0f), true);
   ctVec3 scale = xform.scale;
   xform.scale = ctVec3(1.0f);
   GetCollisionMeshData(mesh, result, false, false, scale);
   uint32_t surface = GetSurfaceHashForPrimitive(node);
   result->shapeSettings = ctPhysicsShapeConvexHull(result->points.Data(),
                                                    (uint32_t)result->points.Count(),
                                                    surface,
                                                    xform,
                                                    node.translation);
   return result;
}

ctGltf2ModelCollision* ctGltf2Model::GetTriangleCollision(const cgltf_node& node,
                                                          bool absoluteTransform) {
   cgltf_mesh* mesh = GetCollisionMeshForNode(node);
   if (!mesh) {
      ctDebugError("COLLISION NODE %s HAS NO MESH!", node.name);
      return NULL;
   }
   ctGltf2ModelCollision* result = new ctGltf2ModelCollision();
   ctVec3 halfExtents = ctVec3(0.5f);
   ctVec3 centerOfMass = ctVec3();
   ctTransform xform = GetCollisionTransform(node, absoluteTransform, ctVec3(0.0f), true);
   ctVec3 scale = xform.scale;
   xform.scale = ctVec3(1.0f);
   GetCollisionMeshData(mesh, result, true, true, scale);
   uint32_t surface = GetSurfaceHashForPrimitive(node);
   result->shapeSettings = ctPhysicsShapeMesh(result->points.Data(),
                                              (uint32_t)result->points.Count(),
                                              result->indices.Data(),
                                              (uint32_t)result->indices.Count(),
                                              surface,
                                              result->materialIndices.Data(),
                                              (uint32_t)result->materialIndices.Count(),
                                              result->materialHashes.Data(),
                                              (uint32_t)result->materialHashes.Count(),
                                              xform,
                                              node.translation);
   return result;
}

ctResults ctGltf2Model::CreateCollisionForCompoundHash(uint32_t hash) {
   /* gather shapes */
   ctDynamicArray<ctPhysicsShapeSettings> shapeList;
   int32_t boneAssociation = -1;
   for (size_t i = 0; i < subshapes.Count(); i++) {
      if (subshapes[i]->compoundNodeNameHash == hash) {
         shapeList.Append(subshapes[i]->shapeSettings);
      }
      if (i == 0) { boneAssociation = subshapes[i]->boneAssociation; }
   }

   ctPhysicsShapeSettings rootSettings;
   if (shapeList.Count() == 1) { /* single shape */
      rootSettings = shapeList[0];
   } else {
      rootSettings = ctPhysicsShapeCompound(shapeList.Count(), shapeList.Data());
   }
   ctGltf2ModelCollisionCompound* compound = new ctGltf2ModelCollisionCompound();
   ctGltf2ModelCollisionBake* bake = new ctGltf2ModelCollisionBake();
   ctPhysicsBakeShape(physicsEngine, rootSettings, bake->bytes);
   uint32_t bakeHash = bake->bytes.xxHash32();
   if (!collisionBakeGroups.Exists(bakeHash)) {
      collisionBakeGroups.Insert(bakeHash, bake);
   } else {
      delete bake;
   }
   compound->bakeHash = bakeHash;
   compound->parentBone = boneAssociation;
   collisionsByGroupHash.Insert(hash, compound);
   return CT_SUCCESS;
}

ctResults ctGltf2Model::CreateCollisionFromConvex(uint32_t surfaceHash) {
   /* todo: visual mesh to convex hull */
   return ctResults();
}

ctResults ctGltf2Model::CreateCollisionFromMesh(uint32_t surfaceHash) {
   /* todo: visual mesh to collision mesh */
   return ctResults();
}

ctResults ctGltf2Model::CreateCollisionFromRig(ctGltf2ModelPhysicsMode mode,
                                               uint32_t surfaceHashOverride) {
   /* Create all subshapes */
   bool absolute = mode == CT_GLTF2MODEL_PHYS_COMPOUND;
   for (size_t i = 0; i < gltf.nodes_count; i++) {
      const cgltf_node& node = gltf.nodes[i];
      if (!(isNodeCollision(node.name) || isNodeBlockout(node.name))) { continue; }
      ctGltf2ModelCollisionType collisionType = isNodeBlockout(node.name)
                                                  ? CT_GLTF2MODEL_COLLISION_TRI
                                                  : GetNodeCollisionType(node.name);
      /* create shape*/
      ctGltf2ModelCollision* pShape = NULL;
      switch (collisionType) {
         case CT_GLTF2MODEL_COLLISION_BOX:
            pShape = GetBoxCollision(node, absolute);
            break;
         case CT_GLTF2MODEL_COLLISION_SPHERE:
            pShape = GetSphereCollision(node, absolute);
            break;
         case CT_GLTF2MODEL_COLLISION_PILL:
            pShape = GetCapsuleCollision(node, absolute);
            break;
         case CT_GLTF2MODEL_COLLISION_TRI:
            pShape = GetTriangleCollision(node, absolute);
            break;
         case CT_GLTF2MODEL_COLLISION_CONVEX:
            pShape = GetConvexCollision(node, absolute);
            break;
         default: ctAssert(0); break;
      }
      if (!pShape) { continue; }

      /* apply surface override */
      if (surfaceHashOverride != 0) {
         pShape->shapeSettings.surfaceTypeHash = surfaceHashOverride;
         if (!pShape->materialHashes.isEmpty()) {
            pShape->materialHashes.Clear();
            pShape->materialHashes.Append(surfaceHashOverride);
         }
         for (size_t j = 0; j < pShape->materialIndices.Count(); j++) {
            pShape->materialIndices[j] = 0;
         }
      }

      if (mode == CT_GLTF2MODEL_PHYS_COMPOUND) { /* all shapes are merged */
         pShape->compoundNodeNameHash = 1;
         pShape->boneAssociation = -1;
      } else if (node.parent) { /* parented to node/bone/etc */
         pShape->compoundNodeNameHash = ctXXHash32(node.parent->name);
         pShape->boneAssociation = BoneIndexFromGltfNode(node.parent->name);
      } else { /* free-floating collision shape */
         pShape->compoundNodeNameHash = ctXXHash32(node.name);
         if (isNodeBlockout(node.name)) {
            pShape->boneAssociation = BoneIndexFromGltfNode(node.name);
         } else {
            ctDebugError("UNPARENTED COLLISION NODE! %s", node.name);
            return CT_FAILURE_INVALID_PARAMETER;
         }
      }
      subshapes.Append(pShape);
   }

   /* create all compound shapes */
   for (size_t i = 0; i < subshapes.Count(); i++) {
      if (!collisionsByGroupHash.Exists(subshapes[i]->compoundNodeNameHash)) {
         CT_RETURN_FAIL(
           CreateCollisionForCompoundHash(subshapes[i]->compoundNodeNameHash));
      }
   }

   return CT_SUCCESS;
}

void ctGltf2Model::SavePhysicsData() {
   /* write bake */
   for (auto it = collisionBakeGroups.GetIterator(); it; it++) {
      it.Value()->writtenOffset = (uint32_t)finalPhysBake.Count();
      finalPhysBake.Append(it.Value()->bytes);
   }

   /* write collision shapes*/
   for (auto it = collisionsByGroupHash.GetIterator(); it; it++) {
      ctModelCollisionShape shape = ctModelCollisionShape();
      shape.boneIndex = it.Value()->parentBone;
      ctGltf2ModelCollisionBake** ppBake =
        collisionBakeGroups.FindPtr(it.Value()->bakeHash);
      ctAssert(ppBake);
      shape.bakeSize = (uint32_t)(*ppBake)->bytes.Count();
      shape.bakeOffset = (uint32_t)(*ppBake)->writtenOffset;
      finalCollisions.Append(shape);
   }
   model.physics.shapeCount = (uint32_t)finalCollisions.Count();
   model.physics.shapes = finalCollisions.Data();
   model.physics.bake.size = finalPhysBake.Count();
   model.physics.bake.data = finalPhysBake.Data();
}

ctResults ctGltf2Model::ExtractPhysics(ctGltf2ModelPhysicsMode mode,
                                       uint32_t surfaceHashOverride) {
   ctDebugLog("Extracting Physics...");

   /* Setup Physics Engine */
   ctPhysicsEngineDesc desc = ctPhysicsEngineDesc();
   ctPhysicsEngineStartup(physicsEngine, desc);

   /* Collision */
   if (mode == CT_GLTF2MODEL_PHYS_CONVEX) { /* convex hull from visual mesh */
      CreateCollisionFromConvex(surfaceHashOverride);
   } else if (mode == CT_GLTF2MODEL_PHYS_MESH) { /* collision mesh from visual mesh */
      CreateCollisionFromMesh(surfaceHashOverride);
   } else { /* use physics rig setup */
      CreateCollisionFromRig(mode, surfaceHashOverride);
   }

   /* Create Binary Save */
   SavePhysicsData();

   /* Shutdown Physics Engine */
   ctPhysicsEngineShutdown(physicsEngine);

   return CT_SUCCESS;
}