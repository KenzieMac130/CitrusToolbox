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

#include "utilities/Common.h"
#include "renderer/KeyLime.hpp"
#include "core/EngineCore.hpp"
#include "renderer/vulkan/VkKeyLimeCore.hpp"

ctResults ctKeyLimeRenderer::Startup() {
   vkKeyLime = new ctVkKeyLimeCore();
   return vkKeyLime->ModuleStartup(Engine);
};

ctResults ctKeyLimeRenderer::Shutdown() {
   const ctResults results = vkKeyLime->ModuleShutdown();
   delete vkKeyLime;
   return results;
}
ctResults ctKeyLimeRenderer::CreateGeometry(ctHandle* pHandleOut,
                                            const char* resolvedPath) {
   ctKeyLimeCreateGeometryDesc desc = ctKeyLimeCreateGeometryDesc();
   ctFile file;
   file.Open(resolvedPath, CT_FILE_OPEN_READ);
   ctDynamicArray<uint8_t> fileContents = {};
   file.GetBytes(fileContents);
   file.Close();
   if (fileContents.Count() < sizeof(ctKeyLimeGeometryHeader)) {
      return CT_FAILURE_CORRUPTED_CONTENTS;
   }
   ctKeyLimeGeometryHeader* pHeader = (ctKeyLimeGeometryHeader*)fileContents.Data();
   if (!ctCStrNEql(pHeader->magic, "GPU0", 4)) { return CT_FAILURE_CORRUPTED_CONTENTS; }
   desc.header = *pHeader;
   desc.blob = fileContents.Data();
   desc.blobSize = fileContents.Count();
   return CreateGeometry(pHandleOut, desc);
}
ctResults ctKeyLimeRenderer::CreateGeometry(ctHandle* pHandleOut,
                                            const ctKeyLimeCreateGeometryDesc& desc) {
   return vkKeyLime->CreateGeometry(pHandleOut, desc);
}
ctResults ctKeyLimeRenderer::UpdateGeometry(ctHandle handle,
                                            const ctKeyLimeCreateGeometryDesc& desc) {
   return vkKeyLime->UpdateGeometry(handle);
}
ctResults ctKeyLimeRenderer::DestroyGeometry(ctHandle handle) {
   return vkKeyLime->DestroyGeometry(handle);
}
ctResults ctKeyLimeRenderer::CreateMaterial(ctHandle* pHandleOut,
                                            const ctKeyLimeMaterialDesc& desc) {
   return vkKeyLime->CreateMaterial(pHandleOut, desc);
}
ctResults ctKeyLimeRenderer::UpdateMaterial(ctHandle handle,
                                            const ctKeyLimeMaterialDesc& desc) {
   return vkKeyLime->UpdateMaterial(handle, desc);
}
ctResults ctKeyLimeRenderer::DestroyMaterial(ctHandle handle) {
   return vkKeyLime->DestroyMaterial(handle);
}
ctResults ctKeyLimeRenderer::CreateTransformPool(ctHandle* pHandleOut,
                                                 const ctKeyLimeTransformsDesc& desc) {
   return vkKeyLime->CreateTransforms(pHandleOut, desc);
}
ctResults ctKeyLimeRenderer::UpdateTransformPool(ctHandle handle,
                                                 const ctKeyLimeTransformsDesc& desc) {
   return vkKeyLime->UpdateTransforms(handle, desc);
}
ctResults ctKeyLimeRenderer::DestroyTransformPool(ctHandle handle) {
   return vkKeyLime->DestroyTransforms(handle);
}
ctResults ctKeyLimeRenderer::CreateGeoInstance(ctHandle* pHandleOut,
                                               const ctKeyLimeInstanceDesc& desc) {
   return vkKeyLime->CreateGeoInstance(pHandleOut, desc);
}
ctResults ctKeyLimeRenderer::UpdateGeoInstance(ctHandle handle,
                                               const ctKeyLimeInstanceDesc& desc) {
   return vkKeyLime->UpdateGeoInstance(handle, desc);
}
ctResults ctKeyLimeRenderer::DestroyGeoInstance(ctHandle handle) {
   return vkKeyLime->DestroyGeoInstance(handle);
}
ctResults ctKeyLimeRenderer::CreateTexture(ctHandle* pHandleOut,
                                           const char* resolvedPath) {
   return CT_SUCCESS;
}
ctResults ctKeyLimeRenderer::DestroyTexture(ctHandle handle) {
   return vkKeyLime->DestroyTexture(handle);
}
ctResults ctKeyLimeRenderer::UpdateCamera(const ctKeyLimeCameraDesc& cameraDesc) {
   return vkKeyLime->UpdateCamera(cameraDesc);
}
ctResults ctKeyLimeRenderer::RenderFrame() {
   return vkKeyLime->Render();
};