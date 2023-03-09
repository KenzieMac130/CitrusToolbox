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
#include "AuditionEditor.hpp"
#include "middleware/ImguiIntegration.hpp"
#include "core/EngineCore.hpp"
#include "core/Translation.hpp"
#include "core/Settings.hpp"
#include "interact/InteractionEngine.hpp"
#include "LiveSync.hpp"

#include "spaces/Compiler.hpp"
#include "spaces/AssetBrowser.hpp"
#include "spaces/ModuleInspector.hpp"
#include "spaces/asseteditors/AssetEditorBase.hpp"
#include "spaces/asseteditors/ModelImport.hpp"
#include "spaces/asseteditors/TextureImport.hpp"
#include "spaces/asseteditors/TextEditor.hpp"
#include "spaces/HexViewer.hpp"

ctResults ctAuditionEditor::Startup() {
   ctSettingsSection* settings = Engine->Settings->GetOrCreateSection("Audition", 1);
   settings->BindInteger(&isHidden,
                         false,
                         true,
                         "IsEditorHidden",
                         "Is the editor hidden by default.",
                         CT_SETTINGS_BOUNDS_BOOL);
   settings->BindInteger(&allowEditor,
                         false,
                         true,
                         "AllowEditor",
                         "Allow the user to open the editor.",
                         CT_SETTINGS_BOUNDS_BOOL);

   RegisterModule((ctModuleBase*)Engine->AssetCompiler);
   RegisterModule((ctModuleBase*)Engine->AsyncTasks);
   RegisterModule((ctModuleBase*)Engine->Debug);
   RegisterModule((ctModuleBase*)Engine->FileSystem);
   RegisterModule((ctModuleBase*)Engine->HotReload);
   RegisterModule((ctModuleBase*)Engine->LiveSync);
   RegisterModule((ctModuleBase*)Engine->Im3dIntegration);
   RegisterModule((ctModuleBase*)Engine->ImguiIntegration);
   RegisterModule((ctModuleBase*)Engine->Interact);
   RegisterModule((ctModuleBase*)Engine->JobSystem);
   RegisterModule((ctModuleBase*)Engine->OSEventManager);
   RegisterModule((ctModuleBase*)Engine->PhysXIntegration);
   RegisterModule((ctModuleBase*)Engine->Renderer);
   RegisterModule((ctModuleBase*)Engine->SceneEngine);
   RegisterModule((ctModuleBase*)Engine->Settings);
   RegisterModule((ctModuleBase*)Engine->Translation);
   RegisterModule((ctModuleBase*)Engine->WindowManager);

   pCompilerWindow = new ctAuditionSpaceCompiler(Engine->AssetCompiler);
   pAssetBrowser = new ctAuditionSpaceAssetBrowser();

   ctx = ctAuditionSpaceContext();
   ctx.Editor = this;
   ctx.Engine = Engine;
   ctx.allowGizmo = true;
   return CT_SUCCESS;
}

ctResults ctAuditionEditor::Shutdown() {
   delete pAssetBrowser;
   delete pCompilerWindow;
   for (size_t i = 0; i < pModuleInspectors.Count(); i++) {
      delete pModuleInspectors[i];
   }
   for (size_t i = 0; i < pDynamicSpaces.Count(); i++) {
      delete pDynamicSpaces[i];
   }
   return CT_SUCCESS;
}

const char* ctAuditionEditor::GetModuleName() {
   return "Audition Editor";
}

void ctAuditionEditor::UpdateEditor() {
   if (!allowEditor) { return; }
   float toggleSignal = Engine->Interact->Directory.GetSignal(
     ctInteractPath("actions/debug/enable/velocity"));
   if (toggleSignal > 0.0f) { isHidden = !isHidden; }
   if (isHidden) { return; }

   bool isHotReloadActive = Engine->HotReload->isStarted();
   if (ImGui::BeginMainMenuBar()) {
      if (ImGui::BeginMenu(CT_NC("File"))) {
         if (ImGui::MenuItem(CT_NC("Load Scene"))) {}
         if (ImGui::MenuItem(CT_NC("Reload Scene"))) {}
         if (ImGui::MenuItem(CT_NC("Save All"))) {}
         ImGui::Separator();
         if (ImGui::MenuItem(CT_NC("Preferences"))) {}
         ImGui::Separator();
         if (ImGui::MenuItem(CT_NC("Exit"))) { Engine->Exit(); }
         ImGui::EndMenu();
      }
      if (ImGui::BeginMenu(CT_NC("Assets"))) {
         pAssetBrowser->DoMenu();
         pCompilerWindow->DoMenu();

         ImGui::Separator();
         bool running = Engine->LiveSync->isRunning();
         if (ImGui::MenuItem(CT_NC("Live Sync"), NULL, running)) {
            if (running) {
               Engine->LiveSync->StopServer();
            } else {
               Engine->LiveSync->StartServer();
            }
         }
         ImGui::EndMenu();
      }
      if (ImGui::BeginMenu(CT_NC("Engine"))) {
         for (size_t i = 0; i < pModuleInspectors.Count(); i++) {
            pModuleInspectors[i]->DoMenu();
         }
         ImGui::EndMenu();
      }
      if (ImGui::BeginMenu(CT_NC("Game"))) {
         if (ImGui::MenuItem(CT_NC("Play Scene"))) {}
         if (ImGui::MenuItem(CT_NC("Play Scene Here"))) {}
         if (ImGui::MenuItem(CT_NC("Simulate Scene"))) {}
         ImGui::Separator();
         if (ImGui::MenuItem(CT_NC("Debug Options"))) {}
         ImGui::Separator();
         if (ImGui::MenuItem(CT_NC("Go to Main Menu"))) {}
         ImGui::EndMenu();
      }
      if (ImGui::BeginMenu(CT_NC("View"))) {
         if (ImGui::MenuItem(CT_NC("Hide Editor"), "Ctrl+Grave")) { isHidden = true; }
         ImGui::EndMenu();
      }
      ImGui::EndMainMenuBar();
   }

   /* Poll Uis */
   pCompilerWindow->Poll(ctx);
   pAssetBrowser->Poll(ctx);
   for (size_t i = 0; i < pModuleInspectors.Count(); i++) {
      pModuleInspectors[i]->Poll(ctx);
   }
   for (size_t i = 0; i < pDynamicSpaces.Count(); i++) {
      if (!pDynamicSpaces[i]) { continue; }
      pDynamicSpaces[i]->Poll(ctx);
   }

   GarbageCollectDynamicSpaces();
}

ctResults ctAuditionEditor::GetTypeForPath(const char* path, char dest[32]) {
   ctStringUtf8 ctacPath = path;
   ctacPath += ".ctac";
   ctFile file = ctFile(ctacPath.CStr(), CT_FILE_OPEN_READ);
   if (!file.isOpen()) {
      strncpy(dest, "untracked", 32);
   } else {
      strncpy(dest, "ctac error", 32);
   }

   ctStringUtf8 jsonContents = "";
   file.GetText(jsonContents);
   file.Close();

   ctJSONReader configFile;
   CT_RETURN_FAIL(
     configFile.BuildJsonForPtr(jsonContents.CStr(), jsonContents.ByteLength()));

   ctJSONReadEntry root = ctJSONReadEntry();
   configFile.GetRootEntry(root);
   if (!root.isObject()) { return CT_FAILURE_CORRUPTED_CONTENTS; }

   ctJSONReadEntry typeEntry;
   ctStringUtf8 type;
   CT_RETURN_FAIL(root.GetObjectEntry("type", typeEntry));
   typeEntry.GetString(type);
   memset(dest, 0, 32);
   type.CopyToArray(dest, 32);
   return CT_SUCCESS;
}

ctResults ctAuditionEditor::TryOpenEditorForAsset(const char* path) {
   char type[32];
   memset(type, 0, 32);
   GetTypeForPath(path, type);

   ctAuditionSpaceAssetEditorBase* pNewEditor;
   if (ctCStrNEql(type, "model", 32)) {
      pNewEditor = new ctAuditionSpaceAssetEditorModelImport();
   } else if (ctCStrNEql(type, "texture", 32)) {
      pNewEditor = new ctAuditionSpaceAssetEditorTextureImport();
   } else if (ctCStrNEql(type, "text", 32) || ctCStrNEql(type, "translation", 32) ||
              ctCStrNEql(type, "shader", 32) || ctCStrNEql(type, "lua", 32) ||
              ctCStrNEql(type, "angelscript", 32)) {
      pNewEditor = new ctAuditionSpaceAssetEditorTextEditor();
   } else {
      if (ctCStrNEql(type, "folder", 32)) { return CT_FAILURE_UNKNOWN; }
      if (ctCStrNEql(type, "untracked", 32)) { return CT_FAILURE_UNKNOWN; }
      if (ctCStrNEql(type, "ctac error", 32)) { return CT_FAILURE_UNKNOWN; }
      pNewEditor = new ctAuditionSpaceAssetEditorBase();
   }
   CT_RETURN_FAIL_CLEAN(AddAssetEditor(pNewEditor, path), delete pNewEditor;)
   return CT_SUCCESS;
}

ctResults ctAuditionEditor::TryOpenHexViewerForResource(ctGUID guid) {
   ctStringUtf8 path = Engine->FileSystem->GetDataPath();
   char guidtext[33];
   memset(guidtext, 0, 33);
   guid.ToHex(guidtext);
   path.FilePathAppend(guidtext);
   return TryOpenHexViewerForFile(path.CStr());
}

ctResults ctAuditionEditor::TryOpenHexViewerForFile(const char* path) {
   ctAuditionSpaceHexViewer* pViewer = new ctAuditionSpaceHexViewer(path);
   CT_RETURN_FAIL_CLEAN(AddDynamicSpace(pViewer), delete pViewer;)
   return CT_SUCCESS;
}

void ctAuditionEditor::RegisterModule(ctModuleBase* pModule) {
   pModuleInspectors.Append(new ctAuditionModuleInspector(pModule));
}

void ctAuditionEditor::GarbageCollectDynamicSpaces() {
   for (size_t i = 0; i < pDynamicSpaces.Count(); i++) {
      if (pDynamicSpaces[i] == NULL) { continue; }
      if (pDynamicSpaces[i]->isOpen()) { continue; }
      delete pDynamicSpaces[i];
      pDynamicSpaces[i] = NULL;
   }
}

ctResults ctAuditionEditor::AddDynamicSpace(ctAuditionSpaceBase* pSpace) {
   pSpace->Open();
   for (size_t i = 0; i < pDynamicSpaces.Count(); i++) {
      if (pDynamicSpaces[i] == NULL) { continue; }
      if (ctCStrEql(pDynamicSpaces[i]->GetWindowName(), pSpace->GetWindowName())) {
         ImGui::SetWindowFocus(pDynamicSpaces[i]->GetWindowName());
         return CT_FAILURE_DUPLICATE_ENTRY;
      }
   }
   for (size_t i = 0; i < pDynamicSpaces.Count(); i++) {
      if (pDynamicSpaces[i] == NULL) {
         pDynamicSpaces[i] = pSpace;
         return CT_SUCCESS;
      }
   }
   pDynamicSpaces.Append(pSpace);
   return CT_SUCCESS;
}

ctResults ctAuditionEditor::AddAssetEditor(ctAuditionSpaceAssetEditorBase* editor,
                                           const char* path) {
   editor->SetFilePath(path);
   editor->Load();
   return AddDynamicSpace(editor);
}