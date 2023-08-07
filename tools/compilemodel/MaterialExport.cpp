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

#define MAKEV2(_arr) ctVec4(_arr[0], _arr[1], 0.0f, 1.0f)
#define MAKEV3(_arr) ctVec4(_arr[0], _arr[1], _arr[2], 1.0f)
#define MAKEV4(_arr) ctVec4(_arr[0], _arr[1], _arr[2], _arr[3])

ctResults ctGltf2Model::WriteTextureProp(ctJSONWriter& writer,
                                         const char* name,
                                         cgltf_texture_view* texture,
                                         bool includeXform) {
   ctAssert(texture);
   if (!texture->texture) { return CT_SUCCESS; }
   if (!texture->texture->image) { return CT_SUCCESS; }
   if (!texture->texture->image->uri) { return CT_SUCCESS; }
   CT_RETURN_FAIL(WriteTextureProp(writer, name, texture->texture->image->uri));
   ctStringUtf8 prop = name;
   if (texture->has_transform) {
      prop = name;
      prop += "_xform";
      ctVec4 xform = ctVec4();
      xform.x = texture->transform.offset[0];
      xform.y = texture->transform.offset[1];
      xform.z = texture->transform.scale[0];
      xform.w = texture->transform.scale[1];
      WriteVectorProp(writer, prop.CStr(), xform);
   }
   if (texture->texcoord != 0) {
      prop = name;
      prop += "_coord";
      WriteIntegerProp(writer, prop.CStr(), texture->texcoord);
   }

   return ctResults();
}

ctResults ctGltf2Model::ExtractMaterials() {
   ctDebugLog("Extracting Materials...");
   if (!gltf.materials_count) { return CT_SUCCESS; }

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
      if (material.unlit) {
         writer.WriteString("BASIC_UNLIT");
      } else {
         writer.WriteString("BASIC_PBR");
      }
      /* custom shader type */
      // todo

      writer.DeclareVariable("properties");
      writer.PushObject();

      /* PBR color, metal, rough */
      if (material.has_pbr_metallic_roughness) {
         cgltf_pbr_metallic_roughness& mr = material.pbr_metallic_roughness;
         WriteVectorProp(writer, "base_color", MAKEV4(mr.base_color_factor));
         if (material.alpha_mode == cgltf_alpha_mode_mask) {
            WriteScalarProp(writer, "alpha_cutoff", material.alpha_cutoff);
         }
         if (mr.metallic_factor != 0.0f) {
            WriteScalarProp(writer, "metalness", mr.metallic_factor);
         }
         if (mr.roughness_factor != 0.5f) {
            WriteScalarProp(writer, "roughness", mr.roughness_factor);
         }
         float aoStrength =
           material.occlusion_texture.texture ? material.occlusion_texture.scale : 0.0f;
         WriteTextureProp(writer, "texture_base_color", &mr.base_color_texture, true);
         WriteTextureProp(
           writer, "texture_pbr_data", &mr.metallic_roughness_texture, true);
      }
      if (material.has_ior) { WriteScalarProp(writer, "ior", material.ior.ior); }

      /* Normalmap and AO map */
      WriteTextureProp(writer, "texture_normalmap", &material.normal_texture, true);
      if (material.normal_texture.texture) {
         WriteScalarProp(writer, "normalmap_strength", material.normal_texture.scale);
      }
      if (material.occlusion_texture.texture) {
         WriteScalarProp(writer, "occlusion_strength", material.occlusion_texture.scale);
      }

      /* Emission */
      float emitStrength = 1.0f;
      if (material.has_emissive_strength) {
         emitStrength = material.emissive_strength.emissive_strength;
      }
      ctVec3 emssion = MAKEV3(material.emissive_factor);
      if (length(emssion * emitStrength) > 0.0f) {
         WriteVectorProp(
           writer, "emission", MAKEV3(material.emissive_factor) * emitStrength);
      }
      WriteTextureProp(writer, "texture_emission", &material.emissive_texture, true);

      /* Custom Properties */
      // todo

      /* Custom Textures */
      // todo

      writer.PopObject();
      writer.PopObject();
   }

   writer.PopArray();
   writer.PopObject();

   model.materialSet.data = (uint8_t*)materialText.Data();
   model.materialSet.size = materialText.ByteLength() + 1;
   return CT_SUCCESS;
}