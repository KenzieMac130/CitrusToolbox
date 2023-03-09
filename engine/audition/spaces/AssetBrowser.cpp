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

#include "AssetBrowser.hpp"
#include "system/System.h"

#include "AssetActions.hpp"

ctAuditionSpaceAssetBrowser::ctAuditionSpaceAssetBrowser() {
   directoryValid = false;
}

ctAuditionSpaceAssetBrowser::~ctAuditionSpaceAssetBrowser() {
}

const char* ctAuditionSpaceAssetBrowser::GetTabName() {
   return "Browser";
}

const char* ctAuditionSpaceAssetBrowser::GetWindowName() {
   return "Asset Browser";
}

void ctAuditionSpaceAssetBrowser::OnGui(ctAuditionSpaceContext& ctx) {
   bool needsRefresh = false;
   if (!(ctx.assetBasePath == baseFolder)) {
      baseFolder = ctx.assetBasePath;
      ctx.currentFolder = baseFolder;
      relativeFolder = "";
      needsRefresh = true;
   }
   if (!directoryValid) {
      ctx.currentFolder = baseFolder;
      relativeFolder = "";
      CurrentFromRelative(ctx.currentFolder);
      needsRefresh = true;
   }
   // if (ImGui::BeginChild("##topbar", ImVec2(0, ImGui::GetStyle().ItemSpacing.y))) {
   if (ImGui::Button(CT_NC("Home"))) {
      relativeFolder = "";
      CurrentFromRelative(ctx.currentFolder);
      needsRefresh = true;
   }
   ImGui::SameLine();
   if (ImGui::Button(CT_NC("Up"))) {
      relativeFolder.FilePathPop();
      CurrentFromRelative(ctx.currentFolder);
      needsRefresh = true;
   }
   ImGui::SameLine();
   if (ImGui::InputText("##path",
                        &relativeFolder,
                        ImGuiInputTextFlags_EnterReturnsTrue |
                          ImGuiInputTextFlags_AllowTabInput)) {
      CurrentFromRelative(ctx.currentFolder);
      needsRefresh = true;
   }
   ImGui::SameLine();
   if (ImGui::Button(CT_NC("Refresh"))) { needsRefresh = true; }
   ImGui::SameLine();
   if (ImGui::InputText("##search", &searchKeyword, ImGuiInputTextFlags_AllowTabInput)) {
      ApplySearch();
   }
   //   ImGui::EndChild();
   //} /* ImGuiInputTextFlags_AllowTabInput is a hack to avoid messing up sizing, todo:
   //fix */
   /* name, date modified, type, size */
   if (ImGui::BeginTable("Assets",
                         4,
                         ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY |
                           ImGuiTableFlags_Sortable | ImGuiTableFlags_Reorderable |
                           ImGuiTableRowFlags_Headers)) {
      ImGui::TableSetupColumn(CT_NC("Name"), ImGuiTableColumnFlags_WidthStretch);
      ImGui::TableSetupColumn(CT_NC("Date"), ImGuiTableColumnFlags_WidthFixed);
      ImGui::TableSetupColumn(CT_NC("Type"), ImGuiTableColumnFlags_WidthFixed);
      ImGui::TableSetupColumn(CT_NC("Size"), ImGuiTableColumnFlags_WidthFixed);

      ImGuiTableSortSpecs* pSortSpecs = ImGui::TableGetSortSpecs();
      if (pSortSpecs->SpecsDirty) {
         for (int i = 0; i < pSortSpecs->SpecsCount; i++) {
            int id = pSortSpecs->Specs[i].ColumnIndex;
            if (id == 0) {
               SortByName();
            } else if (id == 1) {
               SortByDate();
            } else if (id == 2) {
               SortByType();
            } else if (id == 3) {
               SortBySize();
            }
            if (pSortSpecs->Specs[i].SortDirection == ImGuiSortDirection_Descending) {
               directories.Reverse();
            }
         }
         pSortSpecs->SpecsDirty = false;
      }

      ImGui::TableHeadersRow();
      for (size_t i = 0; i < 2; i++) { /* folders then files */
         for (size_t j = 0; j < directories.Count(); j++) {
            DirectoryEntry& dir = directories[j];
            if (i == 0 && dir.isFile) { continue; }
            if (i == 1 && !dir.isFile) { continue; }
            if (!dir.passedSearch) { continue; }
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            bool selected = ctx.hoveredResourcePath == dir.path;

            if (ImGui::Selectable(dir.name,
                                  &selected,
                                  ImGuiSelectableFlags_SpanAllColumns |
                                    ImGuiSelectableFlags_AllowItemOverlap |
                                    ImGuiSelectableFlags_AllowDoubleClick)) {
               ctx.hoveredResourcePath = dir.path;
               if (ImGui::IsMouseDoubleClicked(0)) {
                  if (ctCStrNEql(dir.type, "folder", 32)) {
                     relativeFolder.FilePathAppend(dir.name);
                     relativeFolder.FilePathLocalize();
                     CurrentFromRelative(ctx.currentFolder);
                     needsRefresh = true;
                  } else {
                     ctx.selectedAssetPath = dir.path;
                     ctx.Editor->TryOpenEditorForAsset(ctx.selectedAssetPath.CStr());
                  }
               }
            }
            ImGui::TableNextColumn();
            char buff[32];
            struct tm* timeinfo;
            timeinfo = localtime(&dir.date);
            strftime(buff, sizeof(buff), "%b/%d/%Y %H:%M", timeinfo);
            ImGui::Text(buff);
            ImGui::TableNextColumn();
            ctVec4 typeColor;
            if (ctCStrNEql(dir.type, "folder", 32)) {
               typeColor = CT_COLOR_WHITE;
            } else if (ctCStrNEql(dir.type, "untracked", 32)) {
               typeColor = CT_COLOR_GREY;
            } else if (ctCStrNEql(dir.type, "ctac error", 32)) {
               typeColor = CT_COLOR_RED;
            } else {
               typeColor = ctRandomGenerator(ctXXHash32(dir.type)).GetColor();
            }
            ImGui::TextColored(ctVec4ToImGui(typeColor), dir.type);
            ImGui::TableNextColumn();
            if (!ctCStrNEql(dir.type, "folder", 32)) {
               ctFormatBytes(dir.size, buff, 32);
               ImGui::Text(buff);
            }
         }
      }
      ImGui::EndTable();

      if (ImGui::BeginPopupContextItem("##actions")) {
         if (ImGui::MenuItem(CT_NC("New Asset"))) {
            ctAuditionSpaceActionNewAsset* pSpace =
              new ctAuditionSpaceActionNewAsset(relativeFolder.CStr());
            ctResults res = ctx.Editor->AddDynamicSpace(pSpace);
            if (res != CT_SUCCESS) { delete pSpace; }
         }
         if (ImGui::MenuItem(CT_NC("New Folder"))) {
            ctAuditionSpaceActionNewFolder* pSpace =
              new ctAuditionSpaceActionNewFolder(relativeFolder.CStr());
            ctResults res = ctx.Editor->AddDynamicSpace(pSpace);
            if (res != CT_SUCCESS) { delete pSpace; }
         }
         if (ImGui::MenuItem(CT_NC("Move Selected Asset"))) {
            ctAuditionSpaceActionMoveAsset* pSpace =
              new ctAuditionSpaceActionMoveAsset(ctx.selectedAssetPath.CStr());
            ctResults res = ctx.Editor->AddDynamicSpace(pSpace);
            if (res != CT_SUCCESS) { delete pSpace; }
         }
         if (ImGui::MenuItem(CT_NC("Delete Selected Asset"))) {
            ctAuditionSpaceActionDeleteAsset* pSpace =
              new ctAuditionSpaceActionDeleteAsset(ctx.selectedAssetPath.CStr());
            ctResults res = ctx.Editor->AddDynamicSpace(pSpace);
            if (res != CT_SUCCESS) { delete pSpace; }
         }
         ImGui::EndPopup();
      }
   }

   if (needsRefresh) {
      ctx.hoveredResourcePath = "";
      ctx.selectedAssetPath = "";
      RefreshDirectories(ctx.currentFolder.CStr());
   }
}

void ctAuditionSpaceAssetBrowser::CurrentFromRelative(ctStringUtf8& currentFolder) {
   currentFolder = baseFolder;
   currentFolder.FilePathAppend(relativeFolder);
}

void ctAuditionSpaceAssetBrowser::RefreshDirectories(const char* folder) {
   void* ctx = ctSystemOpenDir(folder);
   if (!ctx) {
      directoryValid = false;
      return;
   }
   directories.Clear();
   while (ctSystemNextDir(ctx)) {
      DirectoryEntry dir = DirectoryEntry();
      ctSystemGetDirName(ctx, dir.name, CT_MAX_FILE_PATH_LENGTH);
      if (dir.name[0] == '.') { continue; }
      ctStringUtf8 path = folder;
      path.FilePathAppend(dir.name);
      path.FilePathLocalize();
      path.CopyToArray(dir.path, CT_MAX_FILE_PATH_LENGTH);
      dir.passedSearch = true;
      dir.isFile = ctSystemIsDirFile(ctx);
      dir.size = ctSystemGetDirFileSize(ctx);
      dir.date = ctSystemGetDirDate(ctx);

      if (dir.isFile) {
         ctStringUtf8 path = dir.path;
         if (path.FilePathGetExtension() == ".ctac") { continue; }
         if (path.FilePathGetExtension() == ".ctpi") { continue; }
         path += ".ctac";
         if (ctSystemFileExists(path.CStr())) {
            strncpy(dir.type, "ctac error", 32);
            ctAuditionEditor::GetTypeForPath(dir.path, dir.type);
         } else {
            strncpy(dir.type, "untracked", 32);
            continue;
         }
      } else {
         ctStringUtf8 path = dir.path;
         if (ctCStrNEql(path.FilePathGetName().CStr(), "waf3", 4)) { continue; }
         strncpy(dir.type, "folder", 32);
      }

      directories.Append(dir);
   }
   directoryValid = true;
}

void ctAuditionSpaceAssetBrowser::SortByName() {
   directories.QSort(DirectoryEntry::CmpName);
}

void ctAuditionSpaceAssetBrowser::SortByDate() {
   directories.QSort(DirectoryEntry::CmpDate);
}

void ctAuditionSpaceAssetBrowser::SortByType() {
   directories.QSort(DirectoryEntry::CmpType);
}

void ctAuditionSpaceAssetBrowser::SortBySize() {
   directories.QSort(DirectoryEntry::CmpSize);
}

void ctAuditionSpaceAssetBrowser::ApplySearch() {
   if (searchKeyword.isEmpty()) {
      for (size_t i = 0; i < directories.Count(); i++) {
         directories[i].passedSearch = true;
      }
      return;
   }
   ctStringUtf8 lwrSearch = searchKeyword;
   lwrSearch.ToLower();
   for (size_t i = 0; i < directories.Count(); i++) {
      DirectoryEntry& dir = directories[i];
      ctStringUtf8 lwrName = dir.name;
      if (lwrSearch.CStr()[0] == '.') { lwrName = lwrName.FilePathGetExtension(); }
      lwrName.ToLower();
      if (ctCStrNEql(lwrSearch.CStr(), lwrName.CStr(), lwrSearch.ByteLength())) {
         dir.passedSearch = true;
      } else {
         if (ctCStrNEql(lwrSearch.CStr(), dir.type, lwrSearch.ByteLength())) {
            dir.passedSearch = true;
         } else {
            dir.passedSearch = false;
         }
      }
   }
}

int ctAuditionSpaceAssetBrowser::DirectoryEntry::CmpName(const DirectoryEntry* a,
                                                         const DirectoryEntry* b) {
   return strncmp(a->name, b->name, 32);
}

int ctAuditionSpaceAssetBrowser::DirectoryEntry::CmpDate(const DirectoryEntry* a,
                                                         const DirectoryEntry* b) {
   return (int)(a->date - b->date);
}

int ctAuditionSpaceAssetBrowser::DirectoryEntry::CmpType(const DirectoryEntry* a,
                                                         const DirectoryEntry* b) {
   int ascore = 0;
   int bscore = 0;
   if (ctCStrNEql(a->type, "untracked", 32)) { ascore -= 9000; }
   if (ctCStrNEql(b->type, "untracked", 32)) { bscore -= 9000; }
   if (ctCStrNEql(a->type, "ctac error", 32)) { ascore += 9000; }
   if (ctCStrNEql(b->type, "ctac error", 32)) { bscore += 9000; }
   return strncmp(a->type, b->type, 32) + (ascore - bscore);
}

int ctAuditionSpaceAssetBrowser::DirectoryEntry::CmpSize(const DirectoryEntry* a,
                                                         const DirectoryEntry* b) {
   return (int)(a->size - b->size);
}