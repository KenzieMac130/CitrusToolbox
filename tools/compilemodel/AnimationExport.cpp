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

ctResults ctGltf2Model::ExtractAnimations() {
   ZoneScoped;
   if (gltf.animations_count == 0) { return CT_SUCCESS; }
   ctDebugLog("Extracting Animations...");
   /* for each clip */
   for (size_t animIndex = 0; animIndex < gltf.animations_count; animIndex++) {
      const cgltf_animation& inanim = gltf.animations[animIndex];
      ctModelAnimationClip& outanim = ctModelAnimationClip();
      strncpy(outanim.name, inanim.name, 32);
      outanim.channelStart = (uint32_t)animChannels.Count();

      /* skip empty clips */
      if (inanim.channels_count == 0) { continue; }

      /* for each channel */
      for (size_t chanIndex = 0; chanIndex < inanim.channels_count; chanIndex++) {
         const cgltf_animation_channel& inchan = inanim.channels[chanIndex];
         ctAssert(inchan.target_node);
         ctAssert(inchan.target_node->name);
         ctAssert(inchan.sampler);

         /* get target node hash */
         uint32_t targetNodeHash = 0;
         if (inchan.target_node) {
            /* skip unpreserved nodes (ex: collisions, lod levels) */
            if (!isNodePreserved(inchan.target_node->name)) { continue; }
            targetNodeHash = ctXXHash32(inchan.target_node->name);
         }

         /* add channel for type */
         if (isNodeCustomAnimProp(inchan.target_node->name)) {
            if (inchan.target_path != cgltf_animation_path_type_translation) { continue; }
            char nameBuff[64];
            strncpy(nameBuff, inchan.target_node->name, 64);
            nameBuff[strlen(nameBuff) - 4] = 0;
            AddCustomChannel(*inchan.sampler,
                             ctXXHash32(nameBuff)); /* todo: extract custom props */
            /* todo: test */
            outanim.channelCount++;
         } else {
            switch (inchan.target_path) {
               case cgltf_animation_path_type_translation:
                  AddTranslationChannel(*inchan.sampler, targetNodeHash);
                  outanim.channelCount++;
                  break;
               case cgltf_animation_path_type_rotation:
                  AddRotationChannel(*inchan.sampler, targetNodeHash);
                  outanim.channelCount++;
                  break;
               case cgltf_animation_path_type_scale:
                  AddScaleChannel(*inchan.sampler, targetNodeHash);
                  outanim.channelCount++;
                  break;
               case cgltf_animation_path_type_weights:
                  ctAssert(inchan.target_node->mesh);
                  ctAssert(inchan.target_node->mesh->target_names);
                  outanim.channelCount += AddWeightChannels(
                    *inchan.sampler,
                    inchan.target_node->mesh->weights_count,
                    (const char**)inchan.target_node->mesh->target_names);
                  break;
               default: break;
            }
         }
      }

      /* finalize and add clip */
      outanim.clipLength = GetClipLength(outanim);
      animClips.Append(outanim);
   }

   /* write out arrays */
   model.animation.clipCount = (uint32_t)animClips.Count();
   model.animation.clips = animClips.Data();
   model.animation.channelCount = (uint32_t)animChannels.Count();
   model.animation.channels = animChannels.Data();
   model.animation.scalarCount = (uint32_t)animScalars.Count();
   model.animation.scalars = animScalars.Data();
   return CT_SUCCESS;
}

ctModelAnimationInterpolation
ctGltf2Model::InterpolationFromGltf(cgltf_interpolation_type type) {
   switch (type) {
      case cgltf_interpolation_type_linear: return CT_MODEL_ANIMINTERP_LINEAR;
      case cgltf_interpolation_type_step: return CT_MODEL_ANIMINTERP_STEP;
      default: return CT_MODEL_ANIMINTERP_LINEAR;
   }
}

uint32_t ctGltf2Model::AddTimeKeys(const cgltf_accessor& accessor) {
   uint32_t offset = (uint32_t)animScalars.Count();
   animScalars.Resize(animScalars.Count() + accessor.count);
   CopyAccessorToReserve(accessor,
                         (uint8_t*)&animScalars[offset],
                         TinyImageFormat_R32_SFLOAT,
                         sizeof(float),
                         0);
   return offset;
}

void ctGltf2Model::AddTranslationChannel(const cgltf_animation_sampler& insampler,
                                         uint32_t boneNameHash) {
   ctModelAnimationChannel animChan = ctModelAnimationChannel();
   animChan.interpolation = InterpolationFromGltf(insampler.interpolation);
   animChan.timeScalarOffset = AddTimeKeys(*insampler.input);
   animChan.valueScalarOffset = (uint32_t)animScalars.Count();
   animChan.targetHash = boneNameHash;
   animChan.type = CT_MODEL_ANIMCHAN_BONE_TRANSLATION;
   animChan.keyCount = (uint32_t)insampler.output->count;
   animScalars.Resize(animScalars.Count() + (size_t)animChan.keyCount * 3);
   CopyAccessorToReserve(*insampler.output,
                         (uint8_t*)&animScalars[animChan.valueScalarOffset],
                         TinyImageFormat_R32G32B32_SFLOAT,
                         sizeof(float) * 3,
                         0);
   animChannels.Append(animChan);
}

void ctGltf2Model::AddRotationChannel(const cgltf_animation_sampler& insampler,
                                      uint32_t boneNameHash) {
   ctModelAnimationChannel animChan = ctModelAnimationChannel();
   animChan.interpolation = InterpolationFromGltf(insampler.interpolation);
   animChan.timeScalarOffset = AddTimeKeys(*insampler.input);
   animChan.valueScalarOffset = (uint32_t)animScalars.Count();
   animChan.targetHash = boneNameHash;
   animChan.type = CT_MODEL_ANIMCHAN_BONE_ROTATION;
   animChan.keyCount = (uint32_t)insampler.output->count;
   animScalars.Resize(animScalars.Count() + (size_t)animChan.keyCount * 4);
   CopyAccessorToReserve(*insampler.output,
                         (uint8_t*)&animScalars[animChan.valueScalarOffset],
                         TinyImageFormat_R32G32B32A32_SFLOAT,
                         sizeof(float) * 4,
                         0);
   animChannels.Append(animChan);
}

void ctGltf2Model::AddScaleChannel(const cgltf_animation_sampler& insampler,
                                   uint32_t boneNameHash) {
   ctModelAnimationChannel animChan = ctModelAnimationChannel();
   animChan.interpolation = InterpolationFromGltf(insampler.interpolation);
   animChan.timeScalarOffset = AddTimeKeys(*insampler.input);
   animChan.valueScalarOffset = (uint32_t)animScalars.Count();
   animChan.targetHash = boneNameHash;
   animChan.type = CT_MODEL_ANIMCHAN_BONE_SCALE;
   animChan.keyCount = (uint32_t)insampler.output->count;
   animScalars.Resize(animScalars.Count() + (size_t)animChan.keyCount * 3);
   CopyAccessorToReserve(*insampler.output,
                         (uint8_t*)&animScalars[animChan.valueScalarOffset],
                         TinyImageFormat_R32G32B32_SFLOAT,
                         sizeof(float) * 3,
                         0);
   animChannels.Append(animChan);
}

/* gltf combines all weights into a single channel citrus expects them seperate */
uint32_t ctGltf2Model::AddWeightChannels(const cgltf_animation_sampler& insampler,
                                         size_t morphCount,
                                         const char** names) {
   ctModelAnimationChannel animChanBase = ctModelAnimationChannel();
   animChanBase.interpolation = InterpolationFromGltf(insampler.interpolation);
   animChanBase.timeScalarOffset = AddTimeKeys(*insampler.input);
   animChanBase.type = CT_MODEL_ANIMCHAN_MORPH_FACTOR;
   animChanBase.keyCount = (uint32_t)insampler.input->count;
   ctDynamicArray<float> tmpOut;
   tmpOut.Resize(insampler.output->count);
   CopyAccessorToReserve(*insampler.output,
                         (uint8_t*)tmpOut.Data(),
                         TinyImageFormat_R32_SFLOAT,
                         sizeof(float),
                         0);
   /* for each morph */
   for (uint32_t morphIdx = 0; morphIdx < morphCount; morphIdx++) {
      /* setup channel */
      ctModelAnimationChannel animChan = animChanBase;
      animChan.valueScalarOffset = (uint32_t)animScalars.Count();
      animChan.targetHash = ctXXHash32(names[morphIdx]);
      for (uint32_t i = 0; i < animChan.keyCount; i++) {
         animScalars.Append(tmpOut[(i * morphCount) + morphIdx]);
      }
      animChannels.Append(animChan);
   }
   return (uint32_t)morphCount;
}

/* custom channels are represented by a local X translation of a node */
void ctGltf2Model::AddCustomChannel(const cgltf_animation_sampler& insampler,
                                    uint32_t propNameHash) {
   ctModelAnimationChannel animChan = ctModelAnimationChannel();
   animChan.interpolation = InterpolationFromGltf(insampler.interpolation);
   animChan.timeScalarOffset = AddTimeKeys(*insampler.input);
   animChan.valueScalarOffset = (uint32_t)animScalars.Count();
   animChan.targetHash = propNameHash;
   animChan.type = CT_MODEL_ANIMCHAN_CUSTOM_VALUE;
   ctDynamicArray<ctVec3> tmpOut;
   tmpOut.Resize(insampler.output->count);
   CopyAccessorToReserve(*insampler.output,
                         (uint8_t*)tmpOut.Data(),
                         TinyImageFormat_R32G32B32_SFLOAT,
                         sizeof(ctVec3),
                         0);
   for (uint32_t i = 0; i < tmpOut.Count(); i++) {
      animScalars.Append(tmpOut[i].x);
   }
   animChannels.Append(animChan);
}

/* build the clip length using the bounds of a final series of clips */
float ctGltf2Model::GetClipLength(ctModelAnimationClip& outanim) {
   float max = 0.0f;
   for (uint32_t chanIdx = 0; chanIdx < outanim.channelCount; chanIdx++) {
      const ctModelAnimationChannel& channel =
        animChannels[(size_t)outanim.channelStart + chanIdx];
      for (uint32_t i = 0; i < channel.keyCount; i++) {
         const float timeValue = animScalars[(size_t)channel.timeScalarOffset + i];
         max = ctMax(max, timeValue);
      }
   }
   return max;
}
