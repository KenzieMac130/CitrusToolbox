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

ctResults ctKeyLimeRenderer::Startup() {
   ctDebugLog("Null Rendering Backend...");
   return CT_SUCCESS;
};

ctResults ctKeyLimeRenderer::Shutdown() {
   return CT_SUCCESS;
}

ctResults ctKeyLimeRenderer::LoadGeometryGpu(ctHandle* pHandleOut,
                                             const char* resolvedPath) {
   return CT_SUCCESS;
}
ctResults ctKeyLimeRenderer::CreateGeometry(ctHandle* pHandleOut,
                                            ctKeyLimeCreateGeometryDesc& desc) {
   return CT_SUCCESS;
}
ctResults ctKeyLimeRenderer::UpdateGeometry(ctHandle handle) {
   return CT_SUCCESS;
}
ctResults ctKeyLimeRenderer::DestroyGeometry(ctHandle handle) {
   return CT_SUCCESS;
}
ctResults ctKeyLimeRenderer::CreateMaterial(ctHandle* pHandleOut,
                                            ctKeyLimeMaterialDesc& desc) {
   return CT_SUCCESS;
}
ctResults ctKeyLimeRenderer::UpdateMaterial(ctHandle handle,
                                            ctKeyLimeMaterialDesc& desc) {
   return CT_SUCCESS;
}
ctResults ctKeyLimeRenderer::DestroyMaterial(ctHandle handle) {
   return CT_SUCCESS;
}
ctResults ctKeyLimeRenderer::CreateTransforms(ctHandle* pHandleOut,
                                              ctKeyLimeTransformsDesc& desc) {
   return CT_SUCCESS;
}
ctResults ctKeyLimeRenderer::UpdateTransforms(ctHandle handle,
                                              ctKeyLimeTransformsDesc& desc) {
   return CT_SUCCESS;
}
ctResults ctKeyLimeRenderer::DestroyTransforms(ctHandle handle) {
   return CT_SUCCESS;
}
ctResults ctKeyLimeRenderer::CreateGeoInstance(ctHandle* pHandleOut,
                                               ctKeyLimeInstanceDesc& desc) {
   return CT_SUCCESS;
}
ctResults ctKeyLimeRenderer::UpdateGeoInstance(ctHandle handle,
                                               ctKeyLimeInstanceDesc& desc) {
   return CT_SUCCESS;
}
ctResults ctKeyLimeRenderer::DestroyGeoInstance(ctHandle handle) {
   return CT_SUCCESS;
}
ctResults ctKeyLimeRenderer::LoadTextureKtx(ctHandle* pHandleOut,
                                            const char* resolvedPath) {
   return CT_SUCCESS;
}
ctResults ctKeyLimeRenderer::DestroyTexture(ctHandle handle) {
   return CT_SUCCESS;
}
ctResults ctKeyLimeRenderer::UpdateCamera(const ctKeyLimeCameraDesc& cameraDesc) {
   return CT_SUCCESS;
}

ctResults ctKeyLimeRenderer::RenderFrame() {
   return CT_SUCCESS;
};