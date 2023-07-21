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

/* Vulkan layout guide
 * https://registry.khronos.org/vulkan/specs/1.1-extensions/html/chap15.html#interfaces-resources-layout
 */

#include "StructVulkan.hpp"

ctGPUStructAssembler* ctGPUStructAssemblerNew(ctGPUDevice* pDevice, ctGPUStructType type) {
   return new ctGPUStructAssembler();
}

void ctGPUStructAssemblerDelete(ctGPUDevice* pDevice, ctGPUStructAssembler* pAssembler) {
   delete pAssembler;
}

size_t ctGPUStructGetBufferSize(ctGPUStructAssembler* pAssembler, size_t arrayCount) {
   return ctAlign(pAssembler->size, pAssembler->largestBaseAlignment) * arrayCount;
}

uint16_t GetBaseAlignment(ctGPUStructVariableType type) {
   switch (type) {
      case CT_GPU_SVAR_INT:
      case CT_GPU_SVAR_UINT: return 4;
      case CT_GPU_SVAR_INT_VEC2:
      case CT_GPU_SVAR_UINT_VEC2: return 8;
      case CT_GPU_SVAR_INT_VEC3:
      case CT_GPU_SVAR_UINT_VEC3: return 16;
      case CT_GPU_SVAR_INT_VEC4:
      case CT_GPU_SVAR_UINT_VEC4: return 16;
      case CT_GPU_SVAR_FLOAT: return 4;
      case CT_GPU_SVAR_FLOAT_VEC2: return 8;
      case CT_GPU_SVAR_FLOAT_VEC3: return 16;
      case CT_GPU_SVAR_FLOAT_VEC4: return 16;
      case CT_GPU_SVAR_FLOAT_MATRIX4X4: return 4;
      default: ctAssert(0); return 0;
   }
}

uint32_t GetSize(ctGPUStructVariableType type) {
   switch (type) {
      case CT_GPU_SVAR_INT:
      case CT_GPU_SVAR_UINT: return 4;
      case CT_GPU_SVAR_INT_VEC2:
      case CT_GPU_SVAR_UINT_VEC2: return 8;
      case CT_GPU_SVAR_INT_VEC3:
      case CT_GPU_SVAR_UINT_VEC3: return 12;
      case CT_GPU_SVAR_INT_VEC4:
      case CT_GPU_SVAR_UINT_VEC4: return 16;
      case CT_GPU_SVAR_FLOAT: return 4;
      case CT_GPU_SVAR_FLOAT_VEC2: return 8;
      case CT_GPU_SVAR_FLOAT_VEC3: return 12;
      case CT_GPU_SVAR_FLOAT_VEC4: return 16;
      case CT_GPU_SVAR_FLOAT_MATRIX4X4: return 4 * 4 * 4;
      default: ctAssert(0); return 0;
   }
}

uint32_t ctGPUStructDefineVariable(ctGPUStructAssembler* pAssembler, ctGPUStructVariableType type) {
   ctAssert(pAssembler->sizes[ctCStaticArrayLen(pAssembler->sizes) - 1] == 0);
   for (int i = 0; i < ctCStaticArrayLen(pAssembler->offsets); i++) {
      if (pAssembler->sizes[i] == 0) {
         uint16_t alignment = GetBaseAlignment(type);
         uint16_t size = GetSize(type);
         pAssembler->size = ctAlign(pAssembler->size, alignment); /* create padding */
         pAssembler->offsets[i] = pAssembler->size;
         pAssembler->sizes[i] = size;
         pAssembler->size += size;
         if (pAssembler->largestBaseAlignment < alignment) {
            pAssembler->largestBaseAlignment = alignment;
         }
         return i;
      }
   }
   return UINT32_MAX;
}

void ctGPUStructSetVariable(ctGPUStructAssembler* pAssembler,
                            const void* pSrc,
                            void* pDst,
                            uint32_t variableIndex,
                            size_t arrayIndex) {
   uint16_t structSize = ctAlign(pAssembler->size, pAssembler->largestBaseAlignment);
   uint16_t offset = pAssembler->offsets[variableIndex];
   size_t size = pAssembler->sizes[variableIndex];
   uint8_t* pBSrc = (uint8_t*)pSrc;
   uint8_t* pBDst = (uint8_t*)pDst;
   memcpy(&pBDst[structSize * arrayIndex + offset], pSrc, size);
}

void ctGPUStructSetVariableMany(ctGPUStructAssembler* pAssembler,
                                const void* pSrc,
                                void* pDst,
                                uint32_t variableIndex,
                                size_t baseArrayIndex,
                                size_t arrayCount,
                                size_t sourceOffset,
                                size_t sourceStride) {
   uint8_t* pBSrc = (uint8_t*)pSrc;
   for (size_t i = 0; i < arrayCount; i++) {
      ctGPUStructSetVariable(pAssembler,
                             &pBSrc[sourceStride * i + sourceOffset],
                             pDst,
                             variableIndex,
                             i + baseArrayIndex);
   }
}