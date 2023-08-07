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

class CT_API ctAnimCanvas {
public:
   ctAnimCanvas(const class ctAnimSkeleton* templateSkeleton,
                const class ctAnimMorphSet* templateMorphSet = NULL,
                const class ctAnimCustomPropSet* templateCustomProps = NULL);
   ctAnimCanvas(const ctAnimCanvas& other);
   ~ctAnimCanvas();

   void ResetToDefault();
   void ApplyLayer(class ctAnimLayerBase& layer, float weight);

   ctResults CopyToSkeleton(class ctAnimSkeleton* dest);
   ctResults CopyToMorphSet(class ctAnimMorphSet* dest);
   ctResults CopyToCustomPropSet(class ctAnimCustomPropSet* dest);

protected:
   friend class ctAnimLayerBase;
   void ApplyBoneTranslation(uint32_t hash, const ctVec3& value, float weight = 1.0f);
   void ApplyBoneRotation(uint32_t hash, const ctQuat& value, float weight = 1.0f);
   void ApplyBoneScale(uint32_t hash, const ctVec3& value, float weight = 1.0f);
   void ApplyBoneTranslation(const struct ctAnimBone& bone,
                             const ctVec3& value,
                             float weight = 1.0f);
   void ApplyBoneRotation(const struct ctAnimBone& bone,
                          const ctQuat& value,
                          float weight = 1.0f);
   void ApplyBoneScale(const struct ctAnimBone& bone,
                       const ctVec3& value,
                       float weight = 1.0f);
   void ApplyMorph(uint32_t hash, float value, float weight = 1.0f);
   void ApplyProp(uint32_t hash, float value, float weight = 1.0f);

private:
   ctAnimCanvas(int32_t boneCount, int32_t morphCount, int32_t propCount);
   inline int32_t FindBoneIndex(uint32_t hash) {
      for (int32_t i = 0; i < boneCount; i++) {
         if (pBoneHashes[i] == hash) { return i; }
      }
      return -1;
   }
   inline int32_t FindMorphIndex(uint32_t hash) {
      for (int32_t i = 0; i < morphCount; i++) {
         if (pMorphHashes[i] == hash) { return i; }
      }
      return -1;
   }
   inline int32_t FindPropIndex(uint32_t hash) {
      for (int32_t i = 0; i < customPropCount; i++) {
         if (pPropHashes[i] == hash) { return i; }
      }
      return -1;
   }

   size_t allocSize;
   void* pAllocation;
   int32_t boneCount;

   uint32_t* pBoneHashes;
   ctTransform* pCurrentBoneLocalTransforms;

   int32_t morphCount;
   uint32_t* pMorphHashes;
   float* pCurrentMorphValues;

   int32_t customPropCount;
   uint32_t* pPropHashes;
   float* pCurrentPropValues;

   ctTransform* pDefaultBoneLocalTransforms;
   float* pDefaultMorphValues;
   float* pDefaultPropValues;
};