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

#include "Bank.hpp"
#include "formats/model/Model.hpp"

ctAnimBank::ctAnimBank(uint32_t clips, uint32_t channels, uint32_t scalars) {
   clipCount = clips;
   channelCount = channels;
   scalarCount = scalars;
   ctGroupAllocDesc groups[] = {
     {4, clipCount * sizeof(pClips[0]), (void**)&pClips},
     {4, channelCount * sizeof(pChannels[0]), (void**)&pChannels},
     {4, scalarCount * sizeof(pScalars[0]), (void**)&pScalars}};
   pAllocation = ctGroupAlloc(ctCStaticArrayLen(groups), groups, &allocSize);
}

ctAnimBank::ctAnimBank(const ctModel& model) :
    ctAnimBank(model.animation.clipCount,
               model.animation.channelCount,
               model.animation.scalarCount) {
   memcpy(pClips, model.animation.clips, clipCount * sizeof(pClips[0]));
   memcpy(pChannels, model.animation.channels, channelCount * sizeof(pChannels[0]));
   memcpy(pScalars, model.animation.scalars, scalarCount * sizeof(pScalars[0]));
}

ctAnimBank::ctAnimBank(const ctAnimBank& other) :
    ctAnimBank(other.clipCount, other.channelCount, other.scalarCount) {
   memcpy(pAllocation, other.pAllocation, allocSize);
}

ctAnimBank::~ctAnimBank() {
   ctFree(pAllocation);
}

uint32_t ctAnimBank::GetClipCount() const {
   return clipCount;
}

ctAnimClip ctAnimBank::GetClip(uint32_t index) const {
   return ctAnimClip(&pClips[index], this);
}

ctResults ctAnimBank::FindClip(const char* name, ctAnimClip* pClipOut) const {
   for (uint32_t i = 0; i < clipCount; i++) {
      if (ctCStrNEql(name, pClips[i].name, 32)) {
         *pClipOut = GetClip(i);
         return CT_SUCCESS;
      }
   }
   return CT_FAILURE_NOT_FOUND;
}

void ctAnimClip::GetName(char output[32]) {
   strncpy(output, pClip->name, 32);
}

float ctAnimClip::GetLength() {
   return pClip->clipLength;
}

uint32_t ctAnimClip::GetChannelCount() const {
   return pClip->channelCount;
}

ctAnimClipChannel ctAnimClip::GetChannel(uint32_t index) const {
   return ctAnimClipChannel(&pBank->pChannels[pClip->channelStart + index], pBank);
}

ctAnimClipChannelType ctAnimClipChannel::GetType() const {
   return (ctAnimClipChannelType)pChannel->type;
}

uint32_t ctAnimClipChannel::GetTargetHash() const {
   return pChannel->targetHash;
}

ctVec3 ctAnimClipChannel::GetVectorAtTime(float time) const {
   ctAssert(pChannel->type == CT_MODEL_ANIMCHAN_BONE_TRANSLATION ||
            pChannel->type == CT_MODEL_ANIMCHAN_BONE_SCALE);
   ctVec3 left;
   ctVec3 right;
   float f = GetScalarGroupAtTime(time, 3, left.data, right.data);
   return lerp(left, right, f);
}

ctQuat ctAnimClipChannel::GetRotationAtTime(float time) const {
   ctAssert(pChannel->type == CT_MODEL_ANIMCHAN_BONE_ROTATION);
   ctQuat left;
   ctQuat right;
   float f = GetScalarGroupAtTime(time, 4, left.data, right.data);
   return ctQuatSlerp(left, right, f);
}

float ctAnimClipChannel::GetFloatAtTime(float time) const {
   ctAssert(pChannel->type == CT_MODEL_ANIMCHAN_MORPH_FACTOR ||
            pChannel->type == CT_MODEL_ANIMCHAN_CUSTOM_VALUE);
   float left;
   float right;
   float f = GetScalarGroupAtTime(time, 1, &left, &right);
   return ctLerp(left, right, f);
}

float ctAnimClipChannel::GetScalarGroupAtTime(float time,
                                              size_t count,
                                              float* left,
                                              float* right) const {
   /* setup search variables (hopefully CPU registers) */
   const float* pScalars = pBank->pScalars;
   const uint32_t keyCount = pChannel->keyCount;
   const uint32_t timeScalarOffset = pChannel->timeScalarOffset;
   uint32_t rightKey = keyCount - 1;
   uint32_t leftKey = rightKey;
   float leftTime = 0.0f;
   float rightTime = 0.0f;
   /* linearly search (yes, we are in only a few cachelines) for left and right key */
   for (uint32_t i = 0; i < keyCount; i++) {
      /* we can assume the first hit that is greater or equal is our right key */
      if (pScalars[timeScalarOffset + i] >= time) {
         rightKey = i;
         rightTime = pScalars[timeScalarOffset + rightKey];
         /* the left key will be the same as the right if right key is the first */
         if (i == 0) {
            leftKey = rightKey;
         } else { /* otherwise the left key will always come just one before */
            leftKey = rightKey - 1;
         }
         leftTime = pScalars[timeScalarOffset + leftKey];
         break;
      }
   }

   /* write out values */
   memcpy(left,
          &pScalars[pChannel->valueScalarOffset + (leftKey * count)],
          sizeof(float) * count);
   memcpy(right,
          &pScalars[pChannel->valueScalarOffset + (rightKey * count)],
          sizeof(float) * count);

   /* setup factor */
   if (pChannel->interpolation == CT_MODEL_ANIMINTERP_LINEAR) {
      return ctClamp(
        (time - leftTime) / ((rightTime + FLT_EPSILON) - leftTime), 0.0f, 1.0f);
   } else {
      return 0.0f;
   }
}