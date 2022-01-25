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

#include "utilities/Common.h"
#include "Device.h"
#include "formats/wad/WADCore.h"
#include "tiny_imageFormat/tinyimageformat.h"

struct ctGPUPipelineBuilder;

typedef void* ctGPUShaderModule;
typedef void* ctGPUPipeline;

enum ctGPUPipelineType {
   CT_GPU_PIPELINE_RASTER,
   CT_GPU_PIPELINE_COMPUTE,
   CT_GPU_PIPELINE_RAYTRACE
};
CT_API ctGPUPipelineBuilder* ctGPUPipelineBuilderNew(ctGPUDevice* pDevice,
                                                     ctGPUPipelineType type);
CT_API void ctGPUPipelineBuilderDelete(ctGPUPipelineBuilder* pBuilder);
CT_API void ctGPUPipelineBuilderReset(ctGPUPipelineBuilder* pBuilder);

/* Shared Options */
enum ctGPUShaderType {
   CT_GPU_SHADER_VERT,
   CT_GPU_SHADER_FRAG,
   CT_GPU_SHADER_GEOMETRY,
   CT_GPU_SHADER_COMPUTE,
   CT_GPU_SHADER_TESS_CONTROL,
   CT_GPU_SHADER_TESS_EVALUATION,
   CT_GPU_SHADER_RAY_GENERATION,
   CT_GPU_SHADER_RAY_ANY_HIT,
   CT_GPU_SHADER_RAY_CLOSEST_HIT,
   CT_GPU_SHADER_RAY_MISS,
   CT_GPU_SHADER_RAY_INTERSECTION,
   CT_GPU_SHADER_CALLABLE,
   CT_GPU_SHADER_TASK,
   CT_GPU_SHADER_MESH,
   CT_GPU_SHADER_COUNT
};
CT_API ctResults ctGPUShaderCreateFromWad(ctGPUDevice* pDevice,
                                          ctGPUShaderModule* pShaderOut,
                                          ctWADReader* pWad,
                                          const char* name,
                                          ctGPUShaderType type);
CT_API void ctGPUShaderSoftRelease(ctGPUDevice* pDevice, ctGPUShaderModule shader);

CT_API ctResults ctGPUPipelineBuilderClearShaders(ctGPUPipelineBuilder* pBuilder);
CT_API ctResults ctGPUPipelineBuilderAddShader(ctGPUPipelineBuilder* pBuilder,
                                               ctGPUShaderType type,
                                               ctGPUShaderModule shader);

/* Raster Options */
enum ctGPUDepthTestMode {
   CT_GPU_DEPTHTEST_FRONT,
   CT_GPU_DEPTHTEST_BEHIND,
   CT_GPU_DEPTHTEST_EQUAL,
   CT_GPU_DEPTHTEST_ALWAYS
};
CT_API ctResults ctGPUPipelineBuilderSetDepthTest(ctGPUPipelineBuilder* pBuilder,
                                                  ctGPUDepthTestMode mode);
CT_API ctResults ctGPUPipelineBuilderSetDepthWrite(ctGPUPipelineBuilder* pBuilder,
                                                   bool enabled);
CT_API ctResults ctGPUPipelineBuilderSetDepthBias(ctGPUPipelineBuilder* pBuilder,
                                                  bool enabled,
                                                  float constantFactor,
                                                  float slopeFactor,
                                                  float clamp);

CT_API ctResults ctGPUPipelineBuilderSetBlendMode(ctGPUPipelineBuilder* pBuilder,
                                                  uint32_t attachmentIndex,
                                                  bool enabled,
                                                  ctColorComponents writeMask,
                                                  ctGPUBlendingMode colorMode,
                                                  ctGPUBlendingMode alphaMode);

enum ctGPUWinding { CT_GPU_WIND_CLOCKWISE, CT_GPU_WIND_COUNTER_CLOCKWISE };
CT_API ctResults ctGPUPipelineBuilderSetWinding(ctGPUPipelineBuilder* pBuilder,
                                                ctGPUWinding winding);

enum ctGPUFillMode { CT_GPU_FILL_SOLID, CT_GPU_FILL_LINES, CT_GPU_FILL_POINTS };
CT_API ctResults ctGPUPipelineBuilderSetFillMode(ctGPUPipelineBuilder* pBuilder,
                                                 ctGPUFillMode fill);

typedef int32_t ctGPUFaceMask;
CT_API ctResults ctGPUPipelineBuilderSetFaceCull(ctGPUPipelineBuilder* pBuilder,
                                                 ctGPUFaceMask cull);
enum ctGPUTopology {
   CT_GPU_TOPOLOGY_TRIANGLE_LIST,
   CT_GPU_TOPOLOGY_TRIANGLE_STRIP,
   CT_GPU_TOPOLOGY_POINT_LIST,
   CT_GPU_TOPOLOGY_LINE_LIST,
   CT_GPU_TOPOLOGY_LINE_STRIP
};
CT_API ctResults ctGPUPipelineBuilderSetTopology(ctGPUPipelineBuilder* pBuilder,
                                                 ctGPUTopology topology,
                                                 bool primativeRestart);
CT_API ctResults ctGPUPipelineBuilderSetTessPoints(ctGPUPipelineBuilder* pBuilder,
                                                   int32_t points);

CT_API ctResults ctGPUPipelineBuilderSetMSAA(ctGPUPipelineBuilder* pBuilder,
                                             ctGPUSampleCounts samples,
                                             bool sampleShading,
                                             bool alphaToCoverage,
                                             bool alphaToOne);

enum ctGPUStencilOps {
   CT_GPU_STENCIL_OP_KEEP,
   CT_GPU_STENCIL_OP_ZERO,
   CT_GPU_STENCIL_OP_REPLACE,
   CT_GPU_STENCIL_OP_INCREMENT_CLAMP,
   CT_GPU_STENCIL_OP_DECREMENT_CLAMP,
   CT_GPU_STENCIL_OP_INCREMENT_INVERT,
   CT_GPU_STENCIL_OP_INCREMENT_WRAP,
   CT_GPU_STENCIL_OP_DECREMENT_WRAP
};
enum ctGPUStencilTest {
   CT_GPU_STENCIL_TEST_NEVER,
   CT_GPU_STENCIL_TEST_LESS,
   CT_GPU_STENCIL_TEST_EQUAL,
   CT_GPU_STENCIL_TEST_LESS_EQUAL,
   CT_GPU_STENCIL_TEST_GREATER,
   CT_GPU_STENCIL_TEST_NOT_EQUAL,
   CT_GPU_STENCIL_TEST_GREATER_EQUAL,
   CT_GPU_STENCIL_TEST_ALWAYS
};
CT_API ctResults ctGPUPipelineBuilderSetStencil(ctGPUPipelineBuilder* pBuilder,
                                                bool enable,
                                                ctGPUFaceMask face,
                                                ctGPUStencilOps failOp,
                                                ctGPUStencilOps passOp,
                                                ctGPUStencilOps depthFailOp,
                                                ctGPUStencilTest testMode,
                                                uint32_t compareMask,
                                                uint32_t writeMask,
                                                uint32_t referenceValue);

enum ctGPUDynamicState {
   CT_GPU_DYNAMICSTATE_VIEWPORT,         /* Default: true */
   CT_GPU_DYNAMICSTATE_SCISSOR,          /* Default: true */
   CT_GPU_DYNAMICSTATE_LINE_WIDTH,       /* Default: false */
   CT_GPU_DYNAMICSTATE_DEPTH_BIAS,       /* Default: false */
   CT_GPU_DYNAMICSTATE_BLEND_CONSTANTS,  /* Default: false */
   CT_GPU_DYNAMICSTATE_DEPTH_BOUNDS,     /* Default: false */
   CT_GPU_DYNAMICSTATE_STENCIL_COMPARE,  /* Default: false */
   CT_GPU_DYNAMICSTATE_STENCIL_WRITE,    /* Default: false */
   CT_GPU_DYNAMICSTATE_STENCIL_REFERENCE /* Default: false */
};
CT_API ctResults ctGPUPipelineBuilderEnableDynamicState(ctGPUPipelineBuilder* pBuilder,
                                                        ctGPUDynamicState state);

CT_API ctResults ctGPUPipelineBuilderSetAttachments(ctGPUPipelineBuilder* pBuilder,
                                                    TinyImageFormat depthFormat,
                                                    uint32_t colorCount,
                                                    TinyImageFormat* colorFormats);

/* Compile Pipelines */
CT_API ctResults ctGPUPipelineCreate(ctGPUDevice* pDevice,
                                     ctGPUPipelineBuilder* pBuilder,
                                     ctGPUPipeline* pPipeline);

CT_API void ctGPUPipelineDestroy(ctGPUDevice* pDevice, ctGPUPipeline pipeline);