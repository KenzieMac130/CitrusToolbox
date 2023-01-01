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

#include "Compiler.hpp"
#include "audition/AssetCompilerBootstrap.hpp"

ctAuditionSpaceCompiler::ctAuditionSpaceCompiler(ctAssetCompilerBootstrap* pCompiler) {
   this->pCompiler = pCompiler;
}

ctAuditionSpaceCompiler::~ctAuditionSpaceCompiler() {
}

const char* ctAuditionSpaceCompiler::GetTabName() {
   return "Compiler";
}

const char* ctAuditionSpaceCompiler::GetWindowName() {
   return "Asset Compiler";
}

void ctAuditionSpaceCompiler::OnGui(ctAuditionSpaceContext& ctx) {
   const float footer_height_to_reserve =
     ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
   if (ImGui::BeginChild("ConsoleScrollRegion",
                         ImVec2(0, -footer_height_to_reserve),
                         false,
                         ImGuiWindowFlags_HorizontalScrollbar)) {
      pCompiler->AcquireOutputLock();
      ImGui::TextUnformatted(pCompiler->GetOutputText());
      pCompiler->ReleaseOutputLock();
   }
   ImGui::EndChild();

   if (ImGui::Button("Clear")) {
      pCompiler->ClearResults();
      ctDebugLog("Cleared Results...");
   }
   ImGui::SameLine();
   if (ImGui::Button("Compile")) {
      pCompiler->StartCompiler();
      ctDebugLog("Compiling...");
   }
}
