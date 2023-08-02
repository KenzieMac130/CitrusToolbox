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

#include "LayerBase.hpp"
#include "Canvas.hpp"

void ctAnimLayerBase::EntryPoint(ctAnimCanvas* pDest, float weight) {
   _masterWeight = weight;
   _pDest = pDest;
   OnApply();
}

void ctAnimLayerBase::ApplyBoneTranslation(const char* name,
                                           const ctVec3& value,
                                           float weight) {
   ApplyBoneTranslation(ctXXHash32(name), value, weight);
}

void ctAnimLayerBase::ApplyBoneRotation(const char* name,
                                        const ctQuat& value,
                                        float weight) {
   ApplyBoneRotation(ctXXHash32(name), value, weight);
}

void ctAnimLayerBase::ApplyBoneScale(const char* name,
                                     const ctVec3& value,
                                     float weight) {
   ApplyBoneScale(ctXXHash32(name), value, weight);
}

void ctAnimLayerBase::ApplyMorph(const char* name, float value, float weight) {
   ApplyMorph(ctXXHash32(name), value, weight);
}

void ctAnimLayerBase::ApplyProp(const char* name, float value, float weight) {
   ApplyProp(ctXXHash32(name), value, weight);
}

void ctAnimLayerBase::ApplyBoneTranslation(uint32_t hash,
                                           const ctVec3& value,
                                           float weight) {
   _pDest->ApplyBoneTranslation(hash, value, _masterWeight * weight);
}

void ctAnimLayerBase::ApplyBoneRotation(uint32_t hash,
                                        const ctQuat& value,
                                        float weight) {
   _pDest->ApplyBoneRotation(hash, value, _masterWeight * weight);
}

void ctAnimLayerBase::ApplyBoneScale(uint32_t hash, const ctVec3& value, float weight) {
   _pDest->ApplyBoneScale(hash, value, _masterWeight * weight);
}

void ctAnimLayerBase::ApplyMorph(uint32_t hash, float value, float weight) {
   _pDest->ApplyMorph(hash, value, _masterWeight * weight);
}

void ctAnimLayerBase::ApplyProp(uint32_t hash, float value, float weight) {
   _pDest->ApplyProp(hash, value, _masterWeight * weight);
}

int32_t ctAnimLayerBase::GetOutputSkeletonBoneCount() {
   return _pDest->boneCount;
}

void ctAnimLayerBase::ApplyBoneTranslation(const ctAnimBone& bone,
                                           const ctVec3& value,
                                           float weight) {
   _pDest->ApplyBoneTranslation(bone, value, _masterWeight * weight);
}

void ctAnimLayerBase::ApplyBoneRotation(const ctAnimBone& bone,
                                        const ctQuat& value,
                                        float weight) {
   _pDest->ApplyBoneRotation(bone, value, _masterWeight * weight);
}

void ctAnimLayerBase::ApplyBoneScale(const ctAnimBone& bone,
                                     const ctVec3& value,
                                     float weight) {
   _pDest->ApplyBoneScale(bone, value, _masterWeight * weight);
}
