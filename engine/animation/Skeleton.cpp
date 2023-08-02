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

#include "Skeleton.hpp"
#include "formats/model/Model.hpp"

ctAnimSkeleton::ctAnimSkeleton(uint32_t count) {
   ZoneScoped;
   dirty = false;
   boneCount = count;

   // clang-format off
   ctGroupAllocDesc groups[] = {
     {4, count * sizeof(pGraph[0]), (void**)&pGraph},
     {CT_ALIGNMENT_QUAT, count * sizeof(pLocalTransforms[0]), (void**)&pLocalTransforms},
     {CT_ALIGNMENT_MAT4, count * sizeof(pModelMatrices[0]), (void**)&pModelMatrices},
     {CT_ALIGNMENT_MAT4, count * sizeof(pInveseBindMatrices[0]), (void**)&pInveseBindMatrices},
     {4, count * sizeof(pHashes[0]), (void**)&pHashes},
     {4, count * sizeof(pNames[0]), (void**)&pNames}
   };
   // clang-format on
   pAllocation = ctGroupAlloc(ctCStaticArrayLen(groups), groups, &allocSize);
   memset(pAllocation, 0, allocSize);
}

ctAnimSkeleton::ctAnimSkeleton(const ctAnimSkeleton& base) :
    ctAnimSkeleton(base.boneCount) {
   ZoneScoped;
   memcpy(pAllocation, base.pAllocation, base.allocSize);
}

ctAnimSkeleton::ctAnimSkeleton(const ctModel& model) :
    ctAnimSkeleton(model.skeleton.boneCount) {
   ZoneScoped;
   memcpy(pGraph, model.skeleton.graphArray, sizeof(pGraph[0]) * boneCount);
   memcpy(pLocalTransforms,
          model.skeleton.transformArray,
          sizeof(pLocalTransforms[0]) * boneCount);
   memcpy(pInveseBindMatrices,
          model.skeleton.inverseBindArray,
          sizeof(pInveseBindMatrices[0]) * boneCount);
   memcpy(pHashes, model.skeleton.hashArray, sizeof(pHashes[0]) * boneCount);
   memcpy(pNames, model.skeleton.nameArray, sizeof(pNames[0]) * boneCount);
   dirty = true;
   FlushBoneTransforms();
}

ctAnimSkeleton::~ctAnimSkeleton() {
   ctFree(pAllocation);
}

int32_t ctAnimSkeleton::FindBoneByName(uint32_t hashName) const {
   ZoneScoped;
   for (int32_t i = 0; i < boneCount; i++) {
      if (pHashes[i] == hashName) { return i; }
   }
   return -1;
}

void ctAnimSkeleton::GetBoneName(ctAnimBone bone, char output[32]) {
   ctAssert(bone.isValid());
   ctAssert(bone.index < boneCount);
   memcpy(output, pNames[bone.index].name, 32);
}

ctAnimBone ctAnimSkeleton::GetBoneParent(const ctAnimBone bone) {
   ctAssert(bone.isValid());
   ctAssert(bone.index < boneCount);
   return pGraph[bone.index].parent;
}

ctAnimBone ctAnimSkeleton::GetBoneFirstChild(ctAnimBone bone) {
   ctAssert(bone.isValid());
   ctAssert(bone.index < boneCount);
   return pGraph[bone.index].firstChild;
}

ctAnimBone ctAnimSkeleton::GetBoneNextSibling(ctAnimBone bone) {
   ctAssert(bone.isValid());
   ctAssert(bone.index < boneCount);
   return pGraph[bone.index].nextSibling;
}

ctMat4 ctAnimSkeleton::GetModelMatrix(ctAnimBone bone) const {
   ctAssert(bone.isValid());
   ctAssert(bone.index < boneCount);
   return pModelMatrices[bone.index];
}

ctMat4 ctAnimSkeleton::GetLocalMatrix(ctAnimBone bone) const {
   ctAssert(bone.isValid());
   ctAssert(bone.index < boneCount);
   ctMat4 out;
   ctMat4FromTransform(out, pLocalTransforms[bone.index]);
   return out;
}

ctMat4 ctAnimSkeleton::GetInverseBindMatrix(ctAnimBone bone) const {
   ctAssert(bone.isValid());
   ctAssert(bone.index < boneCount);
   return pInveseBindMatrices[bone.index];
}

ctTransform ctAnimSkeleton::GetLocalTransform(ctAnimBone bone) const {
   ctAssert(bone.isValid());
   ctAssert(bone.index < boneCount);
   return pLocalTransforms[bone.index];
}

ctTransform ctAnimSkeleton::GetModelTransform(ctAnimBone bone) const {
   ctAssert(bone.isValid());
   ctAssert(bone.index < boneCount);
   ctTransform xform;
   ctMat4AwkwardDecompose(pModelMatrices[bone.index], xform);
   return xform;
}

ctTransform ctAnimSkeleton::GetInverseBindTransform(ctAnimBone bone) const {
   ctAssert(bone.isValid());
   ctAssert(bone.index < boneCount);
   ctTransform xform;
   ctMat4AwkwardDecompose(pInveseBindMatrices[bone.index], xform);
   return xform;
}

void ctAnimSkeleton::SetLocalTransform(ctAnimBone bone, const ctTransform& transform) {
   ctAssert(bone.isValid());
   ctAssert(bone.index < boneCount);
   pLocalTransforms[bone.index] = transform;
   dirty = true;
}

/* go down the tree starting at each root and recusively calculate child transforms */
void ctAnimSkeleton::RecurseBoneUpdate(const ctMat4& parentModel, ctAnimBone bone) {
   ctMat4 model = parentModel * GetLocalMatrix(bone);
   pModelMatrices[bone.index] = model;
   for (ctAnimBoneChildIterator it = ctAnimBoneChildIterator(*this, bone); it;
        it.Next(*this)) {
      RecurseBoneUpdate(model, it.GetBone());
   }
}

void ctAnimSkeleton::FlushBoneTransforms() {
   ZoneScoped;
   if (!dirty) { return; }
   dirty = false;
   /* recurse for each root */
   for (int32_t i = 0; i < boneCount; i++) {
      if (pGraph[i].parent == -1) { RecurseBoneUpdate(ctMat4Identity(), i); }
   }
}

void ctAnimSkeleton::ToModelMatrixArray(ctMat4* matrices, uint32_t count) {
   ZoneScoped;
   ctAssert(matrices);
   ctAssert((int32_t)count <= boneCount);
   memcpy(matrices, pModelMatrices, sizeof(matrices[0]) * count);
}

void ctAnimSkeleton::ToInverseBindMatrixArray(ctMat4* matrices, uint32_t count) {
   ZoneScoped;
   ctAssert(matrices);
   ctAssert((int32_t)count <= boneCount);
   memcpy(matrices, pInveseBindMatrices, sizeof(matrices[0]) * count);
}