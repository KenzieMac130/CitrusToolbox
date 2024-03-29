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

#include "utilities/Common.h"
#include "gpu/Buffer.h"
#include "gpu/Texture.h"

CT_API ctResults ctGPUExternalBufferCreateLoadCPU(ctGPUDevice* pDevice,
                                                  ctGPUExternalBufferPool* pPool,
                                                  ctGPUExternalBuffer** ppBuffer,
                                                  ctGPUExternalBufferCreateLoadInfo* pInfo);

CT_API ctResults ctGPUExternalTextureCreateLoadCPU(ctGPUDevice* pDevice,
                                                   ctGPUExternalTexturePool* pPool,
                                                   ctGPUExternalTexture** ppTexture,
                                                   const char* debugName,
                                                   int32_t desiredBinding,
                                                   ctGPUExternalTexture* pPlaceholder,
                                                   ctGPUExternalTextureType type,
                                                   ctGPUAssetIdentifier* identifier);