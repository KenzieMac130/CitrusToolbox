#include "TextureImport.hpp"
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

const char* ctAuditionSpaceAssetEditorTextureImport::GetAssetTypeName() {
   return "Texture";
}

const char* FormatStrings[] = {"RGBA8",
                               "RGBA16",
                               "RGBA32",
                               "BC1",
                               "BC2",
                               "BC3",
                               "BC4",
                               "BC4_S",
                               "BC5",
                               "BC5_S",
                               "BC6H",
                               "BC7"};

void ctAuditionSpaceAssetEditorTextureImport::OnEditor() {
   DoUIArgCombo(
     "format", CT_NC("Format"), 3, ctCStaticArrayLen(FormatStrings), FormatStrings);
   bool useMips = DoUIArgBool("use_mipmaps", CT_NC("Use Mipmaps"), true);
   if (!useMips) { ImGui::BeginDisabled(); }
   DoUIArgInt("mipmap_count", CT_NC("Number of Mipmaps"), 8, 0, CT_MAX_MIP_LEVELS);
   if (!useMips) { ImGui::EndDisabled(); }
}