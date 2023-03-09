#include "TextureImport.hpp"
#include "TextEditor.hpp"
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

const char* ctAuditionSpaceAssetEditorTextEditor::GetAssetTypeName() {
   return "Text";
}

ctResults ctAuditionSpaceAssetEditorTextEditor::OnLoad(ctJSONReader& configFile) {
   ctFile file = ctFile(assetFilePath, CT_FILE_OPEN_READ);
   if (!file.isOpen()) { return CT_FAILURE_INACCESSIBLE; }
   file.GetText(contents);
   file.Close();
   canSave = true;
   return CT_SUCCESS;
}

ctResults ctAuditionSpaceAssetEditorTextEditor::OnSave(ctJSONWriter& configFile) {
   if (!canSave) { return CT_FAILURE_UNKNOWN; }
   ctFile file = ctFile(assetFilePath, CT_FILE_OPEN_WRITE);
   file.WriteRaw(contents.Data(), 1, contents.ByteLength());
   file.Close();
   return CT_SUCCESS;
}

void ctAuditionSpaceAssetEditorTextEditor::OnEditor() {
   ImGui::InputTextMultiline("##contents",
                             &contents,
                             ImVec2(-FLT_MIN, -FLT_MIN),
                             ImGuiInputTextFlags_AllowTabInput);
}
