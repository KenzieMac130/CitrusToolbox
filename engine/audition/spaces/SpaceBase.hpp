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

#pragma once

#include "utilities/Common.h"
#include "core/Translation.hpp"
#include "middleware/ImguiIntegration.hpp"
#include "audition/AuditionEditor.hpp"

class CT_API ctAuditionSpaceBase {
public:
   void Poll(ctAuditionSpaceContext& ctx) {
      if (!_Open) { return; }
      if (ImGui::Begin(
            CT_NC(GetWindowName()), &_Open, hasMenu ? ImGuiWindowFlags_MenuBar : 0)) {
         OnGui(ctx);
      }
      ImGui::End();
   }
   void DoMenu() {
      if (ImGui::MenuItem(CT_NC(GetTabName()))) {
         Open();
         ImGui::SetWindowFocus(CT_NC(GetWindowName()));
      }
   }

   virtual const char* GetTabName() = 0;
   virtual const char* GetWindowName() = 0;
   virtual void OnGui(ctAuditionSpaceContext& ctx) = 0;

   inline void Close() {
      _Open = false;
   }
   inline void Open() {
      _Open = true;
   }
   inline bool isOpen() {
      return _Open;
   }

protected:
   bool hasMenu = false;

private:
   bool _Open = false;
};