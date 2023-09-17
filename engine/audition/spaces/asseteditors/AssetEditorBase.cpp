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

#include "AssetEditorBase.hpp"
#include "system/System.h"

ctAuditionSpaceAssetEditorBase::ctAuditionSpaceAssetEditorBase() {
   hasMenu = true;
}

ctAuditionSpaceAssetEditorBase::~ctAuditionSpaceAssetEditorBase() {
}

const char* ctAuditionSpaceAssetEditorBase::GetTabName() {
   return GetAssetTypeName();
}

const char* ctAuditionSpaceAssetEditorBase::GetWindowName() {
   return fileName.CStr();
}

const char* ctAuditionSpaceAssetEditorBase::GetAssetTypeName() {
   return "Base";
}

void ctAuditionSpaceAssetEditorBase::OnGui(ctAuditionSpaceContext& ctx) {
   if (ImGui::BeginMenuBar()) {
      if (ImGui::MenuItem(CT_NC("Save"), "Ctrl + S")) { Save(); }
      if (ImGui::MenuItem(CT_NC("Reload"))) { Load(); }
      if (ImGui::MenuItem(CT_NC("External"))) {
         ctSystemShowFileToDeveloper(assetFilePath.CStr());
      }
      ImGui::EndMenuBar();
   }

   if (ImGui::IsKeyDown(ImGuiKey_S) && ImGui::IsKeyPressed(ImGuiKey_LeftCtrl)) { Save(); }

   if (ImGui::BeginTabBar("Tabs")) {
      if (ImGui::BeginTabItem(CT_NC("Editor"))) {
         OnEditor();
         ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem(CT_NC("Arguments"))) {
         int todelete = -1;
         const float footer_height_to_reserve =
           ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
         if (ImGui::BeginTable("ArgTable",
                               3,
                               ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY |
                                 ImGuiTableFlags_ContextMenuInBody |
                                 ImGuiTableRowFlags_Headers,
                               ImVec2(0, -footer_height_to_reserve))) {
            for (size_t i = 0; i < args.Count(); i++) {
               ImGui::PushID((int)i);
               ImGui::TableNextRow();
               ImGui::TableNextColumn();
               ImGui::InputText(CT_NC("Key"), &args[i].key);
               ImGui::TableNextColumn();
               ImGui::InputText(CT_NC("Value"), &args[i].value);
               ImGui::TableNextColumn();
               if (ImGui::Button(CT_NC("Delete"))) { todelete = (int)i; }
               ImGui::PopID();
            }
            ImGui::EndTable();
         }
         if (todelete >= 0) { args.RemoveAt(todelete); }
         if (ImGui::Button(CT_NC("Add Argument"))) { args.Append(Arg("", "")); }
         ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem(CT_NC("Outputs"))) {
         ImGui::InputText(CT_NC("Nickname"), &nickname);
         if (ImGui::BeginTable("OutputsTable",
                               3,
                               ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY |
                                 ImGuiTableFlags_ContextMenuInBody |
                                 ImGuiTableRowFlags_Headers)) {
            for (size_t i = 0; i < outputs.Count(); i++) {
               ImGui::PushID((int)i);
               char guidtext[33];
               memset(guidtext, 0, 33);
               outputs[i].guid.ToHex(guidtext);
               ImGui::TableNextRow();
               ImGui::TableNextColumn();
               if (ImGui::Selectable(outputs[i].nicknamepostfix.CStr(),
                                     false,
                                     ImGuiSelectableFlags_SpanAllColumns |
                                       ImGuiSelectableFlags_AllowItemOverlap |
                                       ImGuiSelectableFlags_AllowDoubleClick)) {
                  if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                     ctx.Editor->TryOpenHexViewerForResource(outputs[i].guid);
                  }
               }
               ImGui::TableNextColumn();
               ImGui::Text(guidtext);
               ImGui::TableNextColumn();
               if (ImGui::Button(CT_NC("Copy to Clipboard"))) {
                  ImGui::SetClipboardText(guidtext);
               }
               ImGui::PopID();
            }
            ImGui::EndTable();
         }
         ImGui::EndTabItem();
      }
      ImGui::EndTabBar();
   }
}

void ctAuditionSpaceAssetEditorBase::SetFilePath(const char* path) {
   assetFilePath = path;
   fileName = assetFilePath.FilePathGetName();
   fileName += assetFilePath.FilePathGetExtension();
}

ctResults ctAuditionSpaceAssetEditorBase::Load() {
   outputs.Clear();
   args.Clear();
   ctStringUtf8 configPath = assetFilePath;
   configPath += ".ctac";
   ctFile file = ctFile(configPath.CStr(), CT_FILE_OPEN_READ);

   ctStringUtf8 jsonContents = "";
   file.GetText(jsonContents);
   file.Close();

   ctJSONReader configFile;
   CT_RETURN_FAIL(
     configFile.BuildJsonForPtr(jsonContents.CStr(), jsonContents.ByteLength()));

   ctJSONReadEntry root = ctJSONReadEntry();
   configFile.GetRootEntry(root);
   if (!root.isObject()) { return CT_FAILURE_CORRUPTED_CONTENTS; }

   ctJSONReadEntry nicknameEntry;
   if (root.GetObjectEntry("nickname", nicknameEntry) == CT_SUCCESS) {
      nicknameEntry.GetString(nickname);
   } else {
      nickname = "";
   }

   ctJSONReadEntry typeEntry;
   CT_RETURN_FAIL(root.GetObjectEntry("type", typeEntry));
   typeEntry.GetString(savedtypename);

   ctJSONReadEntry guidDict = ctJSONReadEntry();
   root.GetObjectEntry("guids", guidDict);

   for (size_t i = 0; i < guidDict.GetObjectEntryCount(); i++) {
      ctJSONReadEntry guidEntry = ctJSONReadEntry();
      ctStringUtf8 nickname = "";
      CT_RETURN_FAIL(guidDict.GetObjectEntry((int)i, guidEntry, &nickname));

      ctStringUtf8 guidString = "";
      guidEntry.GetString(guidString);
      ctGUID guid = ctGUID(guidString.CStr());

      outputs.Append(Output(nickname, guid));
   }

   ctJSONReadEntry argDict = ctJSONReadEntry();
   root.GetObjectEntry("args", argDict);
   for (size_t i = 0; i < argDict.GetObjectEntryCount(); i++) {
      ctJSONReadEntry valueEntry = ctJSONReadEntry();
      ctStringUtf8 key = "";
      CT_RETURN_FAIL(argDict.GetObjectEntry((int)i, valueEntry, &key));

      ctStringUtf8 valueString = "";
      valueEntry.GetString(valueString);
      args.Append(Arg(key, valueString));
   }

   return OnLoad(configFile);
}

ctResults ctAuditionSpaceAssetEditorBase::Save() {
   ctStringUtf8 jsonContents = "";
   ctJSONWriter configFile = ctJSONWriter();
   configFile.SetStringPtr(&jsonContents);
   configFile.PushObject();

   CT_RETURN_FAIL(OnSave(configFile));

   configFile.DeclareVariable("type");
   configFile.WriteString(savedtypename.CStr());

   if (!nickname.isEmpty()) {
      configFile.DeclareVariable("nickname");
      configFile.WriteString(nickname.CStr());
   }

   configFile.DeclareVariable("args");
   configFile.PushObject();
   for (size_t i = 0; i < args.Count(); i++) {
      configFile.DeclareVariable(args[i].key.CStr());
      configFile.WriteString(args[i].value.CStr());
   }
   configFile.PopObject();

   configFile.DeclareVariable("guids");
   configFile.PushObject();
   for (size_t i = 0; i < outputs.Count(); i++) {
      configFile.DeclareVariable(outputs[i].nicknamepostfix.CStr());
      char guidstr[33];
      memset(guidstr, 0, 33);
      outputs[i].guid.ToHex(guidstr);
      configFile.WriteString(guidstr);
   }
   configFile.PopObject();
   configFile.PopObject();

   ctStringUtf8 configPath = assetFilePath;
   configPath += ".ctac";
   ctFile file = ctFile(configPath.CStr(), CT_FILE_OPEN_WRITE);
   file.WriteRaw((void*)jsonContents.CStr(), jsonContents.ByteLength(), 1);
   file.Close();
   return CT_SUCCESS;
}

ctResults ctAuditionSpaceAssetEditorBase::OnLoad(ctJSONReader& configFile) {
   return CT_SUCCESS;
}

ctResults ctAuditionSpaceAssetEditorBase::OnSave(ctJSONWriter& configFile) {
   return CT_SUCCESS;
}

void ctAuditionSpaceAssetEditorBase::OnEditor() {
   ImGui::Text(
     CT_NCT("AUDITION_NO_ASSET_EDITOR", "Asset of type \"%s\" does not have an editor"),
     savedtypename.CStr());
}

void ctAuditionSpaceAssetEditorBase::SetArg(const char* name, const char* value) {
   for (int i = 0; i < args.Count(); i++) {
      if (args[i].key == name) {
         args[i].value = value;
         return;
      }
   }
   args.Append(Arg(name, value));
}

const char* ctAuditionSpaceAssetEditorBase::GetArg(const char* name,
                                                   const char* defaultValue) {
   for (int i = 0; i < args.Count(); i++) {
      if (args[i].key == name) { return args[i].value.CStr(); }
   }
   return defaultValue;
}

bool ctAuditionSpaceAssetEditorBase::DoUIArgBool(const char* name,
                                                 const char* label,
                                                 bool defaultValue) {
   bool value = ctCStrEql(GetArg(name, defaultValue ? "true" : "false"), "true");
   if (ImGui::Checkbox(label, &value)) {
      if (value) {
         SetArg(name, "true");
      } else {
         SetArg(name, "false");
      }
   }
   return value;
}

int ctAuditionSpaceAssetEditorBase::DoUIArgInt(
  const char* name, const char* label, int defaultValue, int min, int max) {
   const char* fetch = GetArg(name, NULL);
   int value = fetch ? atoi(fetch) : defaultValue;
   if (ImGui::SliderInt(label, &value, min, max)) {
      char buff[32];
      memset(buff, 0, 32);
      snprintf(buff, 32, "%d", value);
      SetArg(name, buff);
   }
   return value;
}

float ctAuditionSpaceAssetEditorBase::DoUIArgFloat(
  const char* name, const char* label, float defaultValue, float min, float max) {
   const char* fetch = GetArg(name, NULL);
   float value = fetch ? (float)atof(fetch) : defaultValue;
   if (ImGui::InputFloat(label, &value)) {
      value = ctClamp(value, min, max);
      char buff[32];
      memset(buff, 0, 32);
      snprintf(buff, 32, "%f", value);
      SetArg(name, buff);
   }
   return value;
}

int ctAuditionSpaceAssetEditorBase::DoUIArgCombo(const char* name,
                                                 const char* label,
                                                 int defaultSlot,
                                                 size_t optionCount,
                                                 const char** options) {
   const char* formatString = GetArg(name, options[defaultSlot]);
   int formatIdx = 0;
   for (int i = 0; i < optionCount; i++) {
      if (ctCStrEql(formatString, options[i])) { formatIdx = i; }
   }

   if (ImGui::BeginCombo(label, options[formatIdx])) {
      for (int i = 0; i < optionCount; i++) {
         bool selected = formatIdx == i;
         if (ImGui::Selectable(options[i], &selected)) {
            formatIdx = i;
            SetArg(name, options[i]);
         }
      }
      ImGui::EndCombo();
   }
   return formatIdx;
}

void ctAuditionSpaceAssetEditorBase::DoUIArgString(const char* name,
                                                   const char* label,
                                                   const char* defaultValue) {
   const char* fetch = GetArg(name, NULL);
   char buffer[4096];
   memset(buffer, 0, 4096);
   if (!fetch) { fetch = defaultValue; }
   snprintf(buffer, 32, "%s", fetch);
   if (ImGui::InputText(label, buffer, 4096)) { SetArg(name, buffer); }
}
