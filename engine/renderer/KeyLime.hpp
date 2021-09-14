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

/* API Main Goal: Keep the interface simple for non-graphics programmers! */

#include "core/ModuleBase.hpp"
#include "utilities/Common.h"

#include "KeyLimeDataTypes.hpp"
#include "TextureLoad.h"

class CT_API ctKeyLimeRenderer : public ctModuleBase {
public:
   ctResults Startup() final;
   ctResults Shutdown() final;

   ctResults CreateGeometry(ctKeyLimeGeometryHandle* pHandleOut, const char* resolvedPath);
   ctResults CreateGeometry(ctKeyLimeGeometryHandle* pHandleOut, const ctKeyLimeCreateGeometryDesc& desc);
   ctResults UpdateGeometry(ctKeyLimeGeometryHandle handle, const ctKeyLimeCreateGeometryDesc& desc);
   ctResults DestroyGeometry(ctKeyLimeGeometryHandle handle);

   ctResults CreateMaterial(ctKeyLimeMaterialHandle* pHandleOut, const ctKeyLimeMaterialDesc& desc);
   ctResults UpdateMaterial(ctKeyLimeMaterialHandle handle, const ctKeyLimeMaterialDesc& desc);
   ctResults DestroyMaterial(ctKeyLimeMaterialHandle handle);

   ctResults CreateTransformPool(ctKeyLimeTransformPoolHandle* pHandleOut, const ctKeyLimeTransformsDesc& desc);
   ctResults UpdateTransformPool(ctKeyLimeTransformPoolHandle handle, const ctKeyLimeTransformsDesc& desc);
   ctResults DestroyTransformPool(ctKeyLimeTransformPoolHandle handle);

   ctResults CreateGeoInstance(ctKeyLimeGeoInstanceHandle* pHandleOut, const ctKeyLimeInstanceDesc& desc);
   ctResults UpdateGeoInstance(ctKeyLimeGeoInstanceHandle handle, const ctKeyLimeInstanceDesc& desc);
   ctResults DestroyGeoInstance(ctKeyLimeGeoInstanceHandle handle);

   ctResults CreateTexture(ctKeyLimeTextureHandle* pHandleOut, const char* resolvedPath);
   ctResults CreateTexture(ctKeyLimeTextureHandle* pHandleOut, const ctTextureLoadCtx& loadCtx);
   ctResults DestroyTexture(ctKeyLimeTextureHandle handle);

   ctResults UpdateCamera(const ctKeyLimeCameraDesc& cameraDesc);

   ctResults RenderFrame();

#ifdef CITRUS_GFX_VULKAN
   class ctVkKeyLimeCore* vkKeyLime;
#endif
};