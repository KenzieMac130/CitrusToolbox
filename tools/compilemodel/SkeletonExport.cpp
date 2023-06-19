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

#include "SkeletonExport.hpp"

bool ctModelExportSkeleton::KeepBoneOfName(const char* name) {
   if (isNodeCollision(name) || isNodeLod(name)) { return false; }
   return true;
}

int32_t ctModelExportSkeleton::FindBoneForName(const char* name) {
   uint32_t hash = ctXXHash32(name);
   for (int32_t i = 0; i < (int32_t)boneNames.Count(); i++) {
      if (boneHashes[i] == hash) { return i; }
   }
   return -1;
};

ctResults ctModelExportSkeleton::Export(const cgltf_data& input,
                                        ctModel& output,
                                        ctModelExportContext& ctx) {
   ctDebugLog("Exporting Skeleton...");
   if (ctx.singleBone) {
      uint32_t nameHash = ctXXHash32("root");
      ctStringUtf8* nameString = new ctStringUtf8("root");
      boneNames.Append(nameString->CStr());
      boneHashes.Append(nameHash);

      ctModelSkeletonBoneTransform xform;
      xform.rotation[3] = 1.0f;
      xform.scale[0] = 1.0f;
      xform.scale[1] = 1.0f;
      xform.scale[2] = 1.0f;
      boneTransforms.Append(xform);

      boneGraph.Append({-1, -1, -1});

      ctModelSkeleton* pSkeleton = &output.skeletonData;
      pSkeleton->boneCount = 1;
      pSkeleton->nameArray = boneNames.Data();
      pSkeleton->transformArray = boneTransforms.Data();
      pSkeleton->graphArray = boneGraph.Data();
      return CT_SUCCESS;
   }

   /* pass one (get all bones needed by name) */
   for (size_t i = 0; i < input.nodes_count; i++) {
      const cgltf_node& node = input.nodes[i];
      if (!KeepBoneOfName(node.name)) { continue; }
      uint32_t nameHash = ctXXHash32(node.name);
      ctStringUtf8* nameString = new ctStringUtf8(node.name);
      boneNames.Append(nameString->CStr());
      boneHashes.Append(nameHash);
   }

   /* pass two */
   for (size_t i = 0; i < input.nodes_count; i++) {
      const cgltf_node& node = input.nodes[i];
      if (!KeepBoneOfName(node.name)) { continue; }
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
      memcpy(xform.translation, translation.data, sizeof(float) * 3);
      memcpy(xform.rotation, rotation.data, sizeof(float) * 4);
      memcpy(xform.scale, scale.data, sizeof(float) * 3);
      boneTransforms.Append(xform);
   }

   /* build a sibling heirarchy */
   for (size_t i = 0; i < input.nodes_count; i++) {
      const cgltf_node& node = input.nodes[i];
      if (!KeepBoneOfName(node.name)) { continue; }
      int32_t parent = -1;
      int32_t firstChild = -1;
      int32_t nextSibling = -1;
      if (node.parent) {
         parent = FindBoneForName(node.parent->name);
         /* find next sibling */
         bool nextIsSibling = false;
         for (size_t j = 0; j < node.parent->children_count; j++) {
            if (nextIsSibling) {
               nextSibling = FindBoneForName(node.parent->children[j]->name);
               break;
            }
            if (node.parent->children[j] == &input.nodes[i]) { nextIsSibling = true; }
         }
      }
      if (node.children) { firstChild = FindBoneForName(node.children[0]->name); }
      boneGraph.Append({parent, firstChild, nextSibling});
   }
   ctModelSkeleton* pSkeleton = &output.skeletonData;
   pSkeleton->boneCount = (uint32_t)boneNames.Count();
   pSkeleton->nameArray = boneNames.Data();
   pSkeleton->transformArray = boneTransforms.Data();
   pSkeleton->graphArray = boneGraph.Data();
   return CT_SUCCESS;
}
