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

#include "Canvas.hpp"
#include "LayerBase.hpp"
#include "Skeleton.hpp"
#include "MorphSet.hpp"
#include "CustomPropSet.hpp"
#include "formats/model/Model.hpp"

ctAnimCanvas::ctAnimCanvas(int32_t bones, int32_t morphs, int32_t props) {
   boneCount = bones;
   morphCount = morphs;
   customPropCount = props;

   // clang-format off
    ctGroupAllocDesc allocs[]{
       { 4, sizeof(pBoneHashes[0]) * boneCount, (void**)&pBoneHashes },
       { CT_ALIGNMENT_QUAT, sizeof(pCurrentBoneLocalTransforms[0]) * boneCount, (void**)&pCurrentBoneLocalTransforms },
       { 4, sizeof(pMorphHashes[0]) * morphCount, (void**)&pMorphHashes },
       { 4, sizeof(pCurrentMorphValues[0])* morphCount, (void**)&pCurrentMorphValues },
       { 4, sizeof(pPropHashes[0]) * customPropCount, (void**)&pPropHashes },
       { 4, sizeof(pCurrentPropValues[0])* customPropCount, (void**)&pCurrentPropValues },
       { CT_ALIGNMENT_QUAT, sizeof(pDefaultBoneLocalTransforms[0]) * boneCount, (void**)&pDefaultBoneLocalTransforms },
       { 4, sizeof(pDefaultMorphValues[0]) * morphCount, (void**)&pDefaultMorphValues },
       { 4, sizeof(pDefaultPropValues[0]) * customPropCount, (void**)&pDefaultPropValues }
    };
   // clang-format on
   pAllocation = ctGroupAlloc(ctCStaticArrayLen(allocs), allocs, &allocSize);
}

ctAnimCanvas::ctAnimCanvas(const ctAnimSkeleton* templateSkeleton,
                           const ctAnimMorphSet* templateMorphSet,
                           const ctAnimCustomPropSet* templateCustomProps) :
    ctAnimCanvas(templateSkeleton ? templateSkeleton->boneCount : 0,
                 templateMorphSet ? templateMorphSet->morphCount : 0,
                 templateCustomProps ? templateCustomProps->propCount : 0) {
   /* initialize from skeleton */
   if (templateSkeleton) {
      memcpy(pBoneHashes, templateSkeleton->pHashes, sizeof(pBoneHashes[0]) * boneCount);
      memcpy(pDefaultBoneLocalTransforms,
             templateSkeleton->pLocalTransforms,
             sizeof(pDefaultBoneLocalTransforms[0]) * boneCount);
   }

   /* initialize from morph */
   if (templateMorphSet) {
      for (int32_t i = 0; i < templateMorphSet->morphCount; i++) {
         pMorphHashes[i] = ctXXHash32(templateMorphSet->pMappings[i].name);
         pDefaultMorphValues[i] = templateMorphSet->pMappings[i].value;
      }
   }

   /* initialize from custom props */
   if (templateCustomProps) {
      for (int32_t i = 0; i < templateCustomProps->propCount; i++) {
         pPropHashes[i] = ctXXHash32(templateCustomProps->pProps[i].name);
         pDefaultPropValues[i] = templateCustomProps->pProps[i].value;
      }
   }

   ResetToDefault();
}

ctAnimCanvas::ctAnimCanvas(const ctAnimCanvas& other) :
    ctAnimCanvas(other.boneCount, other.morphCount, other.customPropCount) {
   memcpy(pAllocation, other.pAllocation, allocSize);
}

ctAnimCanvas::~ctAnimCanvas() {
   ctFree(pAllocation);
}

void ctAnimCanvas::ResetToDefault() {
   // clang-format off
   memcpy(pCurrentBoneLocalTransforms,
          pDefaultBoneLocalTransforms,
          sizeof(ctTransform) * boneCount);
   memcpy(pCurrentMorphValues,
          pDefaultMorphValues,
          sizeof(float) * morphCount);
   memcpy(pCurrentPropValues,
          pDefaultPropValues,
          sizeof(float) * customPropCount);
   // clang-format on
}

void ctAnimCanvas::ApplyLayer(ctAnimLayerBase& layer, float weight) {
   layer.EntryPoint(this, weight);
}

ctResults ctAnimCanvas::CopyToSkeleton(ctAnimSkeleton* dest) {
   if (dest->boneCount != boneCount) { return CT_FAILURE_INVALID_PARAMETER; }
   memcpy(dest->pLocalTransforms,
          pCurrentBoneLocalTransforms,
          sizeof(pCurrentBoneLocalTransforms[0]) * boneCount);
   dest->dirty = true;
   return CT_SUCCESS;
}

ctResults ctAnimCanvas::CopyToMorphSet(ctAnimMorphSet* dest) {
   if (dest->morphCount != morphCount) { return CT_FAILURE_INVALID_PARAMETER; }
   for (int32_t i = 0; i < morphCount; i++) {
      dest->pMappings[i].value = pCurrentMorphValues[i];
   }
   return CT_SUCCESS;
}

ctResults ctAnimCanvas::CopyToCustomPropSet(ctAnimCustomPropSet* dest) {
   if (dest->propCount != customPropCount) { return CT_FAILURE_INVALID_PARAMETER; }
   for (int32_t i = 0; i < customPropCount; i++) {
      dest->pProps[i].value = pCurrentPropValues[i];
   }
   return CT_SUCCESS;
}

void ctAnimCanvas::ApplyBoneTranslation(uint32_t hash,
                                        const ctVec3& value,
                                        float weight) {
   int32_t i = FindBoneIndex(hash);
   if (i < 0) { return; }
   ApplyBoneTranslation(ctAnimBone(i), value, weight);
}

void ctAnimCanvas::ApplyBoneRotation(uint32_t hash, const ctQuat& value, float weight) {
   int32_t i = FindBoneIndex(hash);
   if (i < 0) { return; }
   ApplyBoneRotation(ctAnimBone(i), value, weight);
}

void ctAnimCanvas::ApplyBoneScale(uint32_t hash, const ctVec3& value, float weight) {
   int32_t i = FindBoneIndex(hash);
   if (i < 0) { return; }
   ApplyBoneScale(ctAnimBone(i), value, weight);
}

void ctAnimCanvas::ApplyBoneTranslation(const ctAnimBone& bone,
                                        const ctVec3& value,
                                        float weight) {
   ctVec3& out = pCurrentBoneLocalTransforms[bone.index].translation;
   out += (value - out) * weight;
}

void ctAnimCanvas::ApplyBoneRotation(const ctAnimBone& bone,
                                     const ctQuat& value,
                                     float weight) {
   ctQuat& out = pCurrentBoneLocalTransforms[bone.index].rotation;
   out = ctQuatSlerp(out, value, weight);
}

void ctAnimCanvas::ApplyBoneScale(const ctAnimBone& bone,
                                  const ctVec3& value,
                                  float weight) {
   ctVec3& out = pCurrentBoneLocalTransforms[bone.index].scale;
   out += (value - out) * weight;
}

void ctAnimCanvas::ApplyMorph(uint32_t hash, float value, float weight) {
   int32_t i = FindMorphIndex(hash);
   if (i < 0) { return; }
   float& out = pCurrentMorphValues[i];
   out = ctLerp(out, value, weight);
}

void ctAnimCanvas::ApplyProp(uint32_t hash, float value, float weight) {
   int32_t i = FindPropIndex(hash);
   if (i < 0) { return; }
   float& out = pCurrentPropValues[i];
   out = ctLerp(out, value, weight);
}
