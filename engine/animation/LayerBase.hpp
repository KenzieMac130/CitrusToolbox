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

class ctAnimLayerBase {
public:
   void EntryPoint(class ctAnimCanvas* pDest, float weight);
   virtual void OnApply() = 0;

protected:
   void ApplyBoneTranslation(const char* name, const ctVec3& value, float weight = 1.0f);
   void ApplyBoneRotation(const char* name, const ctQuat& value, float weight = 1.0f);
   void ApplyBoneScale(const char* name, const ctVec3& value, float weight = 1.0f);
   void ApplyMorph(const char* name, float value, float weight = 1.0f);
   void ApplyProp(const char* name, float value, float weight = 1.0f);

   void ApplyBoneTranslation(uint32_t hash, const ctVec3& value, float weight = 1.0f);
   void ApplyBoneRotation(uint32_t hash, const ctQuat& value, float weight = 1.0f);
   void ApplyBoneScale(uint32_t hash, const ctVec3& value, float weight = 1.0f);
   void ApplyMorph(uint32_t hash, float value, float weight = 1.0f);
   void ApplyProp(uint32_t hash, float value, float weight = 1.0f);

   int32_t GetOutputSkeletonBoneCount();
   void ApplyBoneTranslation(const struct ctAnimBone& bone,
                             const ctVec3& value,
                             float weight = 1.0f);
   void ApplyBoneRotation(const struct ctAnimBone& bone,
                          const ctQuat& value,
                          float weight = 1.0f);
   void ApplyBoneScale(const struct ctAnimBone& bone,
                       const ctVec3& value,
                       float weight = 1.0f);

private:
   class ctAnimCanvas* _pDest = NULL;
   float _masterWeight;
};