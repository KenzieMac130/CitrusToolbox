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

#ifdef __cplusplus
extern "C" {
#endif

/* Creates gpu compatible struct types at runtime according to API alignment guidelines */

struct ctGPUStructAssembler;
typedef uint32_t uint32_t;

enum ctGPUStructType { CT_GPU_STRUCT_TYPE_UNIFORM, CT_GPU_STRUCT_TYPE_STORAGE };

enum ctGPUStructVariableType {
   CT_GPU_SVAR_INT,            /* int32_t */
   CT_GPU_SVAR_UINT,           /* uint32_t */
   CT_GPU_SVAR_INT_VEC2,       /* int32_t[2] */
   CT_GPU_SVAR_UINT_VEC2,      /* uint32_t[2] */
   CT_GPU_SVAR_INT_VEC3,       /* int32_t[3] */
   CT_GPU_SVAR_UINT_VEC3,      /* uint32_t[3] */
   CT_GPU_SVAR_INT_VEC4,       /* int32_t[4] */
   CT_GPU_SVAR_UINT_VEC4,      /* uint32_t[4] */
   CT_GPU_SVAR_FLOAT,          /* float */
   CT_GPU_SVAR_FLOAT_VEC2,     /* float[2] or ctVec2 */
   CT_GPU_SVAR_FLOAT_VEC3,     /* float[3] or ctVec3 */
   CT_GPU_SVAR_FLOAT_VEC4,     /* float[4] or ctVec4 or ctQuat */
   CT_GPU_SVAR_FLOAT_MATRIX4x4 /* float[4][4] or ctMat4 */
};

struct ctGPUStructAssembler* ctGPUStructAssemblerNew(struct ctGPUDevice* pDevice,
                                                     ctGPUStructType type);
void ctGPUStructAssemblerDelete(struct ctGPUDevice* pDevice,
                                struct ctGPUStructAssembler* pAssembler);

uint32_t
ctGPUStructDefineVariable(struct ctGPUStructAssembler* pAssembler,
                          ctGPUStructVariableType type);

size_t ctGPUStructGetBufferSize(struct ctGPUStructAssembler* pAssembler,
                                size_t arrayCount);

void ctGPUStructSetVariable(struct ctGPUStructAssembler* pAssembler,
                            const void* pSrc,
                            void* pDst,
                            uint32_t variableIndex,
                            size_t arrayIndex);
void ctGPUStructSetVariableMany(struct ctGPUStructAssembler* pAssembler,
                                const void* pSrc,
                                void* pDst,
                                uint32_t variableIndex,
                                size_t baseArrayIndex,
                                size_t arrayCount,
                                size_t sourceOffset,
                                size_t sourceStride);

#ifdef __cplusplus
}
#endif