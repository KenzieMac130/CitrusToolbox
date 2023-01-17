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

#include "HexViewer.hpp"
#include "imgui/imgui_memory_editor.h"

ctAuditionSpaceHexViewer::ctAuditionSpaceHexViewer(const char* filePath) {
   windowName = filePath;
   windowName = windowName.FilePathGetName();
   windowName += " Hex Viewer";
   pMemEditor = new MemoryEditor();
   pMemEditor->ReadOnly = true;
   ctFile file = ctFile(filePath, CT_FILE_OPEN_READ);
   file.GetBytes(bytes);
   file.Close();
}

ctAuditionSpaceHexViewer::~ctAuditionSpaceHexViewer() {
   delete pMemEditor;
}

const char* ctAuditionSpaceHexViewer::GetTabName() {
   return "Hex Editor";
}

const char* ctAuditionSpaceHexViewer::GetWindowName() {
   return windowName.CStr();
}

void ctAuditionSpaceHexViewer::OnGui(ctAuditionSpaceContext& ctx) {
   pMemEditor->DrawContents(bytes.Data(), bytes.Count());
}