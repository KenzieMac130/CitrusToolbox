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

struct CT_API ctAnimBone {
   inline ctAnimBone() {
      index = -1;
   }
   inline ctAnimBone(int32_t v) {
      index = v;
   }
   inline ctAnimBone(ctAnimBone& other) {
      index = other.index;
   }
   inline bool isValid() const {
      return index >= 0;
   }
   int32_t index;
};

class CT_API ctAnimSkeleton {
public:
   ctAnimSkeleton(uint32_t boneCount);
   ctAnimSkeleton(const ctAnimSkeleton& base);
   ctAnimSkeleton(const struct ctModel& model);
   ~ctAnimSkeleton();

   inline int32_t GetBoneCount() const {
      return boneCount;
   }

   int32_t FindBoneByName(uint32_t hashName) const;
   inline int32_t FindBoneByName(const char* name) const {
      return FindBoneByName(ctXXHash32(name));
   }

   void GetBoneName(ctAnimBone bone, char output[32]);
   ctAnimBone GetBoneParent(ctAnimBone bone);
   ctAnimBone GetBoneFirstChild(ctAnimBone bone);
   ctAnimBone GetBoneNextSibling(ctAnimBone bone);

   ctMat4 GetLocalMatrix(ctAnimBone bone) const;
   ctMat4 GetModelMatrix(ctAnimBone bone) const;
   ctMat4 GetInverseBindMatrix(ctAnimBone bone) const;
   ctTransform GetLocalTransform(ctAnimBone bone) const;
   ctTransform GetModelTransform(ctAnimBone bone) const;
   ctTransform GetInverseBindTransform(ctAnimBone bone) const;

   inline bool NeedsFlush() {
      return dirty;
   }
   void SetLocalTransform(ctAnimBone bone, const ctTransform& transform);
   void FlushBoneTransforms();

   void ToModelMatrixArray(ctMat4* matrices, uint32_t count);
   void ToInverseBindMatrixArray(ctMat4* matrices, uint32_t count);

protected:
   friend class ctAnimCanvas;
   bool dirty;
   int32_t boneCount;
   size_t allocSize;
   void* pAllocation;

   struct ctModelSkeletonBoneGraph* pGraph;
   ctTransform* pLocalTransforms;
   ctMat4* pModelMatrices;
   ctMat4* pInveseBindMatrices;
   uint32_t* pHashes;
   struct ctModelSkeletonBoneName* pNames;

   void RecurseBoneUpdate(const ctMat4& parentModel, ctAnimBone bone);
};

class CT_API ctAnimBoneChildIterator {
public:
   inline ctAnimBoneChildIterator(class ctAnimSkeleton& skeleton, ctAnimBone parent) {
      bone = skeleton.GetBoneFirstChild(parent);
   }
   inline operator bool() const {
      return bone.isValid();
   }
   inline void Next(class ctAnimSkeleton& skeleton) {
      bone = skeleton.GetBoneNextSibling(bone);
   }
   inline ctAnimBone GetBone() {
      return bone;
   }

private:
   friend class ctAnimSkeleton;
   ctAnimBone bone;
};