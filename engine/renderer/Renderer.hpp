/*
   Copyright 2021 MacKenzie Strand

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

#include "core/ModuleBase.hpp"
#include "utilities/Common.h"

/* Opaque handle to a Pipeline Shader Object */
typedef void* ctRenderPipeline;

/* Key Lime Renderer Stuff */

enum ctKeyLimePipelines {
   CT_KEYLIME_PIPELINE_UNDEFINED,
   CT_KEYLIME_PIPELINE_UBER,
   CT_KEYLIME_PIPELINE_IMGUI,
   CT_KEYLIME_PIPELINE_IM3D,
   CT_KEYLIME_PIPELINE_MAX = UINT8_MAX
};

// clang-format off
struct ctKeyLimePipelineDesc {
   ctKeyLimePipelines pipelineTypes;    /* Type of pipeline */
   union {
      struct {                          /* ---Ubershader Options-- */
         bool depthTest : 1;            /* Do Alpha Test */
         bool depthWrite : 1;           /* Do Alpha Writing */
         bool wireframe : 1;            /* Do Wireframe */
         bool isLit : 1;                /* Use Phong Lighting */
         bool isLightmapped : 1;        /* Use Instance Lightmap */
         bool isTransprarent : 1;       /* Alpha Blending */
         bool isAdditive : 1;           /* Additive Blending */
         bool isRefractive : 1;         /* Refractive Blending Pass */
         bool useGlow : 1;              /* Add Glow Color */
         bool useBloom : 1;             /* Output Components to Bloom Buffer */
         bool useReflection : 1;        /* Do Reflection Mapping */
         bool useVertexColor : 1;       /* Multiply Vertex Color */
         bool useBaseTexture : 1;       /* Multiply Base Texture */
         bool useAuxDetailTexture : 1;  /* Auxilary Mult/Add RGB, Alpha Sub -A */
         bool useAuxDistortTexture : 1; /* Auxilary Texture: Displace UV */
         bool useAuxGlowMask : 1;       /* Auxilary Texture: Multiply Glow by RGB */
         bool useAlphaReflectMask : 1;  /* Use Final Alpha to Multiply Reflections */
         bool useAlphaClip : 1;         /* Use Final Alpha to Discard Fragments */
      } uberShader;
   };
};
// clang-format on

class ctRenderer : public ctModuleBase {
public:
   ctResults Startup() final;
   ctResults Shutdown() final;

#ifdef CITRUS_GFX_VULKAN
   class ctVkBackend* vkBackend;
#endif
};