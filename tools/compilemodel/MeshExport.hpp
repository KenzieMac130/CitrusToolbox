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

#pragma once

#include "utilities/Common.h"
#include "ExportPhaseBase.hpp"

#include "formats/model/extensions/RenderMesh.hpp"
#include "tiny_imageFormat/tinyimageformat.h"

class CT_API ctModelExportMesh : public ctModelExportPhaseBase {
public:
   virtual ctResults
   Export(const cgltf_data& input, ctModel& o3utput, ctModelExportContext& ctx);

private:
   ctResults ExtractIndexBuffer(const cgltf_accessor& accessor,
                           ctModelMeshLod& lod,
                           class ctModelExportGeomContainer& geo);
   ctResults ApplyIndexMerge(ctModelMeshLod& lod, class ctModelExportGeomContainer& geo);
   ctResults ExtractAttribute(const cgltf_attribute& attribute,
                         ctModelMeshLod& lod,
                         class ctModelExportGeomContainer& geo);
   ctResults CopyAccessorToReserve(const cgltf_accessor& accessor, void* destination, TinyImageFormat format, size_t stride = 0, size_t offset = 0);
   ctResults CalculateTangents(ctModelMeshLod& lod, class ctModelExportGeomContainer& geo);
};