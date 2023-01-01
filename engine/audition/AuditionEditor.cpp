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
#include "spaces/ModuleInspector.hpp"

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
   return CT_SUCCESS;
}

ctResults ctAuditionEditor::Shutdown() {
   delete pCompilerWindow;
   for (size_t i = 0; i < pModuleInspectors.Count(); i++) {
      delete pModuleInspectors[i];
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
         if (ImGui::MenuItem(CT_NC("Browser"))) {}
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

   ctAuditionSpaceContext ctx = ctAuditionSpaceContext();

   /* Poll Uis */
   pCompilerWindow->Poll(ctx);
   for (size_t i = 0; i < pModuleInspectors.Count(); i++) {
      pModuleInspectors[i]->Poll(ctx);
   }
}

void ctAuditionEditor::RegisterModule(ctModuleBase* pModule) {
   pModuleInspectors.Append(new ctAuditionModuleInspector(pModule));
}