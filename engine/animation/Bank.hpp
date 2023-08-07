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

/* matches ctModelAnimationChannelType */
enum ctAnimClipChannelType {
   CT_ANIMCLIP_BONE_TRANSLATION,
   CT_ANIMCLIP_BONE_ROTATION,
   CT_ANIMCLIP_BONE_SCALE,
   CT_ANIMCLIP_MORPH_FACTOR,
   CT_ANIMCLIP_CUSTOM_VALUE,
};

class CT_API ctAnimClipChannel {
public:
   ctAnimClipChannelType GetType() const;
   uint32_t GetTargetHash() const;
   ctVec3 GetVectorAtTime(float time) const;
   ctQuat GetRotationAtTime(float time) const;
   float GetFloatAtTime(float time) const;

protected:
   inline ctAnimClipChannel(const struct ctModelAnimationChannel* chan,
                            const class ctAnimBank* bank) {
      pChannel = chan;
      pBank = bank;
   }
   friend class ctAnimBank;
   friend class ctAnimClip;
   friend class ctAnimLayerClip;
   const class ctAnimBank* pBank;
   const struct ctModelAnimationChannel* pChannel;
   float GetScalarGroupAtTime(float time, size_t count, float* left, float* right) const;
};

class CT_API ctAnimClip {
public:
   void GetName(char output[32]);
   float GetLength();
   uint32_t GetChannelCount() const;
   ctAnimClipChannel GetChannel(uint32_t index) const;

protected:
   inline ctAnimClip(const struct ctModelAnimationClip* clip,
                     const class ctAnimBank* bank) {
      pClip = clip;
      pBank = bank;
   }
   friend class ctAnimBank;
   friend class ctAnimLayerClip;
   const struct ctModelAnimationClip* pClip;
   const class ctAnimBank* pBank;
};

class CT_API ctAnimBank {
public:
   ctAnimBank(const struct ctModel& model);
   ctAnimBank(const ctAnimBank& other);
   ~ctAnimBank();

   uint32_t GetClipCount() const;
   ctAnimClip GetClip(uint32_t index) const;
   ctResults FindClip(const char* name, ctAnimClip* pClipOut) const;

protected:
   friend ctAnimClipChannel;
   friend ctAnimClip;
   ctAnimBank(uint32_t clipCount, uint32_t channelCount, uint32_t scalarCount);

   size_t allocSize;
   void* pAllocation;

   uint32_t clipCount;
   struct ctModelAnimationClip* pClips;

   uint32_t channelCount;
   struct ctModelAnimationChannel* pChannels;

   uint32_t scalarCount;
   float* pScalars;
};