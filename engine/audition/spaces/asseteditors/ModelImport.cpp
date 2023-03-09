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
#include "ModelImport.hpp"

const char* ctAuditionSpaceAssetEditorModelImport::GetAssetTypeName() {
   return "Model";
}

const char* ModelModes[] {
  "single",       /* basic one mesh models + LODs (common props, foliage, etc) */
  "articulation", /* rigs with bone trees (characters, physics objects, etc) */
  "level",        /* level geometry and spawners */
  "destructable", /* geometry which can fracture (destructable cover) */
  "vat",          /* vertex animation texture model (presimulated effect) */
};

const char* BlastBondModes[] {"exact", "average"};

void ctAuditionSpaceAssetEditorModelImport::OnEditor() {
   int mode = DoUIArgCombo(
     "model_mode", CT_NC("Model Mode"), 0, ctCStaticArrayLen(ModelModes), ModelModes);
   DoUIArgBool("import_curves", CT_NC("Import Curves"), true);
   DoUIArgBool("import_lights", CT_NC("Import Lights"), true);

   /* articulation only */
   if ((mode == 1)) {
      DoUIArgBool("import_bone_animations", CT_NC("Import Animation"), true);
   }

   ImGui::Separator();
   /* not applicable to destructables */
   if (mode != 3) { DoUIArgBool("physics_export", CT_NC("Export Physics"), true); }
   DoUIArgFloat("physics_mass_scale", CT_NC("Mass Scale"), 1.0f, 0.00001f, 1000.0f);
   DoUIArgString("physics_material_override", CT_NC("Surface Override"), "");
   ImGui::Separator();

   /* not applicable to destructables or vats */
   if (mode != 3 && mode != 4) {
      DoUIArgBool("import_mesh_lods", CT_NC("Import Mesh LODs"), true);
      if (mode != 2) {
         DoUIArgBool("import_blend_shapes", CT_NC("Import Blend Shapes"), false);
      }
      DoUIArgBool("import_keep_topology", CT_NC("Preserve Topology"), false);
   }

   /* only destructables */
   if (mode == 3) {
      DoUIArgString("destructable_original", CT_NC("Original Model"), "");
      DoUIArgString("destructable_core", CT_NC("Core Model"), "");
      DoUIArgString("destructable_preset", CT_NC("Destruction Preset"), "");
      DoUIArgCombo("blast_bond_mode",
                   CT_NC("Bond Generation"),
                   0,
                   ctCStaticArrayLen(BlastBondModes),
                   BlastBondModes);
      DoUIArgFloat(
        "blast_max_separation", CT_NC("Max Separation"), 0.01f, FLT_MIN, FLT_MAX);
   }

   /* only vats */
   if (mode == 4) {
      DoUIArgString("vat_tex_lut", CT_NC("VAT Lookup Texture"), "");
      DoUIArgString("vat_tex_position", CT_NC("VAT Position Texture"), "");
      DoUIArgString("vat_tex_rotation", CT_NC("VAT Rotation Texture"), "");
      DoUIArgString("vat_tex_color", CT_NC("VAT Color Texture"), "");
      DoUIArgString("vat_metadata", CT_NC("VAT Metadata"), "");
   }
}