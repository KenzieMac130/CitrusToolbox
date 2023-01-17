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

const char* PhysicsModes[] {"none", "bodies", "articulation", "destructable"};

void ctAuditionSpaceAssetEditorModelImport::OnEditor() {
   DoUIArgBool("import_render_meshes", CT_NC("Import Render Meshes"), true);
   DoUIArgBool("import_materials", CT_NC("Import Materials"), true);
   DoUIArgBool("search_materials", CT_NC("Search for Materials"), true);
   ImGui::Separator();
   DoUIArgBool("import_spawners", CT_NC("Import Spawners"), true);
   DoUIArgBool("import_curves", CT_NC("Import Curves"), true);
   ImGui::Separator();
   int mode = DoUIArgCombo("physics_mode",
                           CT_NC("Physics Mode"),
                           1,
                           ctCStaticArrayLen(PhysicsModes),
                           PhysicsModes);
   if (mode == 0) { ImGui::BeginDisabled(); }
   DoUIArgFloat("physics_mass_scale", CT_NC("Mass Scale"), 1.0f, 0.00001f, 1000.0f);
   DoUIArgString("physics_material_override", CT_NC("Surface Override"), "");
   if (mode == 0) { ImGui::EndDisabled(); }
   ImGui::Separator();
   DoUIArgBool("import_animations", CT_NC("Import Animations"), true);
   DoUIArgBool("import_blend_shapes", CT_NC("Import Blend Shapes"), true);
}
