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
#include "tiny_imageFormat/tinyimageformat.h"

/* Handles all textures */

struct ctGPUExternalTexture;

enum ctGPUExternalTextureType {
   CT_GPU_EXTERN_TEXTURE_TYPE_2D,
   CT_GPU_EXTERN_TEXTURE_TYPE_3D,
   CT_GPU_EXTERN_TEXTURE_TYPE_CUBE
};

struct ctGPUExternalGenerateContext {
   TinyImageFormat format;
   uint32_t width;
   uint32_t height;
   uint32_t depth;
   uint32_t currentMipLevel;
};

typedef void (*ctGPUTextureGenerateFn)(uint8_t* dest,
                                       ctGPUExternalGenerateContext* pCtx,
                                       void* userData);

struct ctGPUExternalTextureCreateInfo {
   const char* debugName;
   ctGPUExternalTextureType type;

   ctGPUExternalSource source;
   union {
      struct {
         TinyImageFormat format;
         uint32_t width;
         uint32_t height;
         uint32_t depth;
         uint32_t mips;

         ctGPUExternalUpdateMode updateMode;
         ctGPUTextureGenerateFn fpGenerationFunction;
         void* pUserData;
      } generate;
      struct {
         ctGPUAssetIdentifier assetIdentifier;
      } load;
   };
};

CT_API ctResults ctGPUExternalTextureCreate(struct ctGPUDevice* pDevice,
                                            ctGPUExternalTexture** ppExternalTexture,
                                            ctGPUExternalTextureCreateInfo* pInfo);
CT_API ctResults ctGPUExternalTextureDestroy(struct ctGPUDevice* pDevice,
                                             ctGPUExternalTexture* ppExternalTexture);
CT_API ctResults ctGPUExternalTextureRequestUpdate(
  struct ctGPUDevice* pDevice, ctGPUExternalTexture* ppExternalTexture);
CT_API ctResults ctGPUExternalTextureGetAccessor(struct ctGPUDevice* pDevice,
                                                 ctGPUExternalTexture* ppExternalTexture,
                                                 ctGPUImageAccessor* pAccessorOut);