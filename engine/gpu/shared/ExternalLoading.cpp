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

#include "ExternalLoading.hpp"
#include "DeviceBase.hpp"

struct ExternBufferLoadCtx {
   ctGPUDeviceBase* pDeviceBase;
   ctGPUAssetIdentifier identifier;
   size_t offset;
   size_t size;
};

void LoadBuffer(uint8_t* dest, size_t size, ExternBufferLoadCtx* pCtx) {
   ctFile file = pCtx->pDeviceBase->OpenAssetFile(&pCtx->identifier);
   if (!file.isOpen()) {
      memset(dest, 0, size);
      delete pCtx;
      return;
   }
   if (pCtx->offset) { file.Seek(pCtx->offset, CT_FILE_SEEK_SET); }
   file.ReadRaw((void*)dest, size, 1);
   file.Close();
   delete pCtx;
}

CT_API ctResults
ctGPUExternalBufferCreateLoadCPU(ctGPUDevice* pDevice,
                                 ctGPUExternalBufferPool* pPool,
                                 ctGPUExternalBuffer** ppBuffer,
                                 ctGPUExternalBufferCreateLoadInfo* pInfo) {
   ExternBufferLoadCtx* pCtx = new ExternBufferLoadCtx();
   pCtx->pDeviceBase = (ctGPUDeviceBase*)pDevice;
   pCtx->identifier = *pInfo->identifier;
   pCtx->offset = pInfo->offset;
   pCtx->size = pInfo->size;
   ctGPUExternalBufferCreateFuncInfo info;
   info.debugName = pInfo->debugName;
   info.desiredBinding = pInfo->desiredBinding;
   info.async = true;
   info.pPlaceholder = NULL;
   info.updateMode = CT_GPU_UPDATE_STATIC;
   info.size = pInfo->size;
   info.generationFunction = (ctGPUBufferGenerateFn)LoadBuffer;
   info.userData = pCtx;
   return ctGPUExternalBufferCreateFunc(pDevice, pPool, ppBuffer, &info);
}

CT_API ctResults ctGPUExternalTextureCreateLoadCPU(ctGPUDevice* pDevice,
                                                   ctGPUExternalTexturePool* pPool,
                                                   ctGPUExternalTexture** ppTexture,
                                                   const char* debugName,
                                                   int32_t desiredBinding,
                                                   ctGPUExternalTexture* pPlaceholder,
                                                   ctGPUExternalTextureType type,
                                                   ctGPUAssetIdentifier* identifier) {
   return CT_API ctResults();
}
