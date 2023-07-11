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

#include "AssetActions.hpp"

ctAuditionSpaceActionNewAsset::ctAuditionSpaceActionNewAsset(const char* relativePath) {
   this->relativePath = relativePath;
   this->relativePath.FilePathUnify();
}

ctAuditionSpaceActionNewAsset::~ctAuditionSpaceActionNewAsset() {
}

const char* ctAuditionSpaceActionNewAsset::GetTabName() {
   return "New Asset";
}

const char* ctAuditionSpaceActionNewAsset::GetWindowName() {
   return "New Asset";
}

struct NewTypeEntry {
   const char* ui;
   const char* cmd;
};

NewTypeEntry NewAssetTypes[] = {{"Scene", "scene"},
                                {"Particle Effect", "particle_effect"}};

void ctAuditionSpaceActionNewAsset::OnGui(ctAuditionSpaceContext& ctx) {
   if (ImGui::BeginCombo(CT_NC("Type"), NewAssetTypes[typeId].ui)) {
      for (int i = 0; i < ctCStaticArrayLen(NewAssetTypes); i++) {
         bool selected = typeId == i;
         if (ImGui::Selectable(NewAssetTypes[i].ui, &selected)) { typeId = i; }
      }
      ImGui::EndCombo();
   }
   ImGui::InputText("Name", &name);
   if (ImGui::Button("Create Asset")) {
      // todo
   }
}

ctAuditionSpaceActionNewFolder::ctAuditionSpaceActionNewFolder(const char* relativePath) {
   this->relativePath = relativePath;
   this->relativePath.FilePathUnify();
}

ctAuditionSpaceActionNewFolder::~ctAuditionSpaceActionNewFolder() {
}

const char* ctAuditionSpaceActionNewFolder::GetTabName() {
   return "New Folder";
}

const char* ctAuditionSpaceActionNewFolder::GetWindowName() {
   return "New Folder";
}

void ctAuditionSpaceActionNewFolder::OnGui(ctAuditionSpaceContext& ctx) {
   ImGui::Text("%s/", relativePath.CStr());
   ImGui::SameLine();
   ImGui::InputText("Folder Name", &name);
   if (ImGui::Button("Create Folder")) {
      // todo
   }
}

ctAuditionSpaceActionDeleteAsset::ctAuditionSpaceActionDeleteAsset(const char* path) {
   this->path = path;
   this->path.FilePathUnify();
}

ctAuditionSpaceActionDeleteAsset::~ctAuditionSpaceActionDeleteAsset() {
}

const char* ctAuditionSpaceActionDeleteAsset::GetTabName() {
   return "Delete Asset";
}

const char* ctAuditionSpaceActionDeleteAsset::GetWindowName() {
   return "Delete Asset";
}

void ctAuditionSpaceActionDeleteAsset::OnGui(ctAuditionSpaceContext& ctx) {
   ImGui::Text("Delete %s?", path);
   if (ImGui::Button("Confirm Delete")) {
      // todo
   }
}

ctAuditionSpaceActionMoveAsset::ctAuditionSpaceActionMoveAsset(const char* srcPath) {
   this->srcPath = srcPath;
   this->srcPath.FilePathUnify();
}

ctAuditionSpaceActionMoveAsset::~ctAuditionSpaceActionMoveAsset() {
}

const char* ctAuditionSpaceActionMoveAsset::GetTabName() {
   return "Move Asset";
}

const char* ctAuditionSpaceActionMoveAsset::GetWindowName() {
   return "Move Asset";
}

void ctAuditionSpaceActionMoveAsset::OnGui(ctAuditionSpaceContext& ctx) {
   ImGui::Text("Moving %s to %s", srcPath.CStr(), ctx.currentFolder.CStr());
   if (ImGui::Button("Confirm Move")) {
      // todo
   }
}