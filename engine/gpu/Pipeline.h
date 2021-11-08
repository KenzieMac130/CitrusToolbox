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

struct ctGPUPipelineBuilder;

typedef void* ctGPUShaderModule;
typedef void* ctGPUPipelineRaster;
typedef void* ctGPUPipelineCompute;
typedef void* ctGPUPipelineRaytrace;

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
                                          int32_t fxSegment,
                                          ctGPUShaderType type);
CT_API void ctGPUShaderSoftRelease(ctGPUDevice* pDevice, ctGPUShaderModule shader);

CT_API ctResults ctGPUPipelineBuilderSetShader(ctGPUPipelineBuilder* pBuilder,
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
                                                  float slopeFactor);
enum ctGPUBlendingMode {
   CT_GPU_BLEND_ADD,
   CT_GPU_BLEND_SUBTRACT,
   CT_GPU_BLEND_MULTIPLY,
   CT_GPU_BLEND_MAX,
   CT_GPU_BLEND_MIN,
   CT_GPU_BLEND_LERP,
   CT_GPU_BLEND_OVERWRITE,
   CT_GPU_BLEND_DISCARD
};
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

enum ctGPUFaceCull {
   CT_GPU_FACECULL_FRONT,
   CT_GPU_FACECULL_BACK,
   CT_GPU_FACECULL_NONE,
   CT_GPU_FACECULL_BOTH
};
CT_API ctResults ctGPUPipelineBuilderSetFaceCull(ctGPUPipelineBuilder* pBuilder,
                                                 ctGPUFaceCull cull);
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

enum ctGPUSampleCounts {
   CT_GPU_SAMPLES_1 = 1,
   CT_GPU_SAMPLES_2 = 2,
   CT_GPU_SAMPLES_4 = 4,
   CT_GPU_SAMPLES_8 = 8,
   CT_GPU_SAMPLES_16 = 16,
   CT_GPU_SAMPLES_32 = 32,
   CT_GPU_SAMPLES_64 = 64
};
CT_API ctResults ctGPUPipelineBuilderSetMSAA(ctGPUPipelineBuilder* pBuilder,
                                             ctGPUSampleCounts samples,
                                             bool sampleShading,
                                             bool alphaToCoverage,
                                             bool alphaToOne);

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

CT_API ctResults ctGPUPipelineBuilderSetRasterTask(ctGPUPipelineBuilder* pBuilder,
                                                   struct ctGPUArchitect* pArchitect,
                                                   const char* name);

/* Compute Options */

/* Compile Pipelines */
CT_API ctResults ctGPUPipelineBuilderGenerateRaster(ctGPUDevice* pDevice,
                                                    ctGPUPipelineBuilder* pBuilder,
                                                    ctGPUPipelineRaster* pPipeline);
CT_API ctResults ctGPUPipelineBuilderGenerateCompute(ctGPUDevice* pDevice,
                                                     ctGPUPipelineBuilder* pBuilder,
                                                     ctGPUPipelineCompute* pPipeline);
CT_API ctResults ctGPUPipelineBuilderGenerateRaytrace(ctGPUDevice* pDevice,
                                                      ctGPUPipelineBuilder* pBuilder,
                                                      ctGPUPipelineRaytrace* pPipeline);

CT_API void ctGPUPipelineRasterDestroy(ctGPUDevice* pDevice,
                                       ctGPUPipelineRaster pipeline);
CT_API void ctGPUPipelineComputeDestroy(ctGPUDevice* pDevice,
                                        ctGPUPipelineRaster pipeline);
CT_API void ctGPUPipelineRaytraceDestroy(ctGPUDevice* pDevice,
                                         ctGPUPipelineRaster pipeline);