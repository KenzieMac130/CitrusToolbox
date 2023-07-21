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

#include "../engine/utilities/Common.h"
#include "CitrusModel.hpp"

ctResults ctGltf2Model::ExtractMaterials() {
   ctJSONWriter writer;
   writer.SetStringPtr(&materialText);

   writer.PushObject();
   writer.DeclareVariable("materials");
   writer.PushArray();

   /* todo */
   for (size_t matidx = 0; matidx < gltf.materials_count; matidx++) {
      writer.PushObject();
      cgltf_material& material = gltf.materials[matidx];

      /* material name */
      writer.DeclareVariable("name");
      writer.WriteString(material.name);

      /* material shader */
      writer.DeclareVariable("shader");
      writer.WriteString("BASIC_PBR");

      writer.DeclareVariable("properties");
      writer.PushObject();

      WriteScalarProp(writer, "foo", material.alpha_cutoff); /* example */
      /* todo: once the renderer is written more */

      writer.PopObject();
      writer.PopObject();
   }

   writer.PopArray();
   writer.PopObject();

   model.materialSet.data = (uint8_t*)materialText.Data();
   model.materialSet.size = materialText.ByteLength() + 1;
   return CT_SUCCESS;
}