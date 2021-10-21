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

/* API Main Goal: Keep the interface simple for non-graphics programmers!*/

#include "core/ModuleBase.hpp"
#include "utilities/Common.h"

#include "KeyLimeDataTypes.hpp"

class CT_API ctKeyLimeRenderer : public ctModuleBase {
public:
   ctResults Startup() final;
   ctResults Shutdown() final;

   ctResults CreateGeometry(ctKeyLimeGeometryReference* pHandleOut,
                            const ctKeyLimeGeometryDesc& desc);
   ctResults GetGeometryState(ctKeyLimeGeometryReference handle);
   ctResults DestroyGeometry(ctKeyLimeGeometryReference handle);

   ctResults CreateTexture(ctKeyLimeTextureReference* pHandleOut,
                           const ctKeyLimeTextureDesc& desc);
   ctResults GetTextureState(ctKeyLimeTextureReference handle);
   ctResults DestroyTexture(ctKeyLimeTextureReference handle);

   // ctResults CreateMaterial(ctKeyLimeMaterialReference* pHandleOut,
   //                         const ctKeyLimeMaterialDesc& desc);
   // ctResults GetMaterialState(ctKeyLimeMaterialReference handle);
   // ctResults UpdateMaterial(ctKeyLimeMaterialReference handle,
   //                         const ctKeyLimeMaterialDesc& desc);
   // ctResults DestroyMaterial(ctKeyLimeMaterialReference handle);
   //
   // ctResults CreateTransformPool(ctKeyLimeTransformPoolReference* pHandleOut,
   //                              const ctKeyLimeTransformsDesc& desc);
   // ctResults GetTransformPoolState(ctKeyLimeTransformPoolReference handle);
   // ctResults UpdateTransformPool(ctKeyLimeTransformPoolReference handle,
   //                              const ctKeyLimeTransformsDesc& desc);
   // ctResults DestroyTransformPool(ctKeyLimeTransformPoolReference handle);
   //
   // ctResults CreateGeoInstance(ctKeyLimeGeoInstanceReference* pHandleOut,
   //                            const ctKeyLimeInstanceDesc& desc);
   // ctResults GetGeoInstanceState(ctKeyLimeGeoInstanceReference handle);
   // ctResults UpdateGeoInstance(ctKeyLimeGeoInstanceReference handle,
   //                            const ctKeyLimeInstanceDesc& desc);
   // ctResults DestroyGeoInstance(ctKeyLimeGeoInstanceReference handle);

   ctResults UpdateCamera(const ctKeyLimeCameraDesc& cameraDesc);

   ctResults RenderFrame();

private:
#ifdef CITRUS_GFX_VULKAN
   class ctVkKeyLimeCore* vkKeyLime;
#endif
};