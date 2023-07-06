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

ctResults ctGltf2Model::ExtractSkeleton() {
   ctDebugLog("Extracting Skeleton...");
   /* pass one (get all bones needed by name) */
   for (size_t i = 0; i < gltf.nodes_count; i++) {
      const cgltf_node& node = gltf.nodes[i];
      if (!isNodePreserved(node.name)) { continue; }
      uint32_t nameHash = ctXXHash32(node.name);
      ctModelSkeletonBoneName name = ctModelSkeletonBoneName();
      strncpy(name.name, node.name, 32);
      boneNames.Append(name);
      boneHashes.Append(nameHash);
   }

   /* pass two (extract transforms) */
   for (size_t i = 0; i < gltf.nodes_count; i++) {
      const cgltf_node& node = gltf.nodes[i];
      if (!isNodePreserved(node.name)) { continue; }
      ctVec3 translation = ctVec3();
      ctQuat rotation = ctQuat();
      ctVec3 scale = ctVec3(1.0f);
      if (!node.has_translation || !node.has_rotation || !node.has_scale) {
         if (node.has_matrix) {
            ctVec3 tmptranslation;
            ctQuat tmprotation;
            ctVec3 tmpscale;
            ctMat4AwkwardDecompose(
              ctMat4(node.matrix), tmptranslation, tmprotation, tmpscale);
            if (!node.has_translation) { translation = tmptranslation; }
            if (!node.has_rotation) { rotation = tmprotation; }
            if (!node.has_scale) { scale = tmpscale; }
         }
      }
      if (node.has_translation) { translation = node.translation; }
      if (node.has_rotation) { rotation = node.rotation; }
      if (node.has_scale) { scale = node.scale; }
      ctModelSkeletonBoneTransform xform = {};
      memcpy(xform.translation.data, translation.data, sizeof(float) * 3);
      memcpy(xform.rotation.data, rotation.data, sizeof(float) * 4);
      memcpy(xform.scale.data, scale.data, sizeof(float) * 3);
      boneTransforms.Append(xform);

      /* also extract inverse world transform for skinning */
      ctModelSkeletonBoneTransform toroot = {};
      ctMat4 worldXform;
      cgltf_node_transform_world(&node, worldXform.data[0]);
      worldXform = ctMat4InverseLossy(worldXform);
      ctMat4AwkwardDecompose(
        worldXform, toroot.translation, toroot.rotation, toroot.scale);
      boneInverseBinds.Append(toroot);
   }

   /* pass three (build a sibling heirarchy) */
   for (size_t i = 0; i < gltf.nodes_count; i++) {
      const cgltf_node& node = gltf.nodes[i];
      if (!isNodePreserved(node.name)) { continue; }
      int32_t parent = -1;
      int32_t firstChild = -1;
      int32_t nextSibling = -1;
      if (node.parent) {
         parent = BoneIndexFromGltfNode(node.parent->name);
         /* find next sibling */
         bool nextIsSibling = false;
         for (size_t j = 0; j < node.parent->children_count; j++) {
            if (nextIsSibling) {
               nextSibling = BoneIndexFromGltfNode(node.parent->children[j]->name);
               break;
            }
            if (node.parent->children[j] == &gltf.nodes[i]) { nextIsSibling = true; }
         }
      }
      if (node.children) { firstChild = BoneIndexFromGltfNode(node.children[0]->name); }
      boneGraph.Append({parent, firstChild, nextSibling});
   }

   /* todo: look into sorting? */

   model.skeleton.boneCount = (uint32_t)boneHashes.Count();
   model.skeleton.graphArray = boneGraph.Data();
   model.skeleton.nameArray = boneNames.Data();
   model.skeleton.transformArray = boneTransforms.Data();
   model.skeleton.inverseBindArray = boneInverseBinds.Data();
   model.skeleton.hashArray = boneHashes.Data();

   return CT_SUCCESS;
}

bool ctGltf2Model::isNodeCollision(const char* name) {
   size_t len = strlen(name);
   if (len < 4) { return false; }
   const char* postfix = &name[len - 4];
   if (ctCStrNEql(postfix, "_CBX", 4) || /* box collision */
       ctCStrNEql(postfix, "_CSP", 4) || /* sphere collision */
       ctCStrNEql(postfix, "_CPL", 4) || /* pill collision */
       ctCStrNEql(postfix, "_CTR", 4) || /* triangle collision */
       ctCStrNEql(postfix, "_CVX", 4))   /* convex collision */
   {
      return true;
   }
   return false;
}

bool ctGltf2Model::isNodeLODLevel(const char* name) {
   size_t len = strlen(name);
   if (len < 5) { return false; }
   const char* postfix = &name[len - 4];
   if (ctCStrNEql(postfix, "LOD", 3)) { /* lod levels LOD1-LOD9 */
      return true;
   }
   return false;
}

bool ctGltf2Model::isNodePath(const char* name) {
   size_t len = strlen(name);
   if (len < 5) { return false; }
   const char* postfix = &name[len - 4];
   if (ctCStrNEql(postfix, "PATH", 4)) { /* lod levels LOD1-LOD9 */
      return true;
   }
   return false;
}

bool ctGltf2Model::isNodePreserved(const char* name) {
   if (isNodeCollision(name) || isNodeLODLevel(name) || isNodePath(name)) {
      return false;
   }
   return true;
}

int32_t ctGltf2Model::BoneIndexFromGltfNode(const char* nodeName) {
   uint32_t hash = ctXXHash32(nodeName);
   for (int32_t i = 0; i < (int32_t)boneHashes.Count(); i++) {
      if (boneHashes[i] == hash) { return i; }
   }
   return -1;
}