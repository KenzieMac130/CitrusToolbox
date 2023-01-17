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
#include "../SpaceBase.hpp"

class CT_API ctAuditionSpaceAssetEditorBase : public ctAuditionSpaceBase {
public:
   ctAuditionSpaceAssetEditorBase();
   ~ctAuditionSpaceAssetEditorBase();

   virtual const char* GetTabName();
   virtual const char* GetWindowName();
   virtual const char* GetAssetTypeName();
   virtual void OnGui(ctAuditionSpaceContext& ctx);

   void SetFilePath(const char* path);
   ctResults Load();
   ctResults Save();

   virtual ctResults OnLoad(ctJSONReader& configFile);
   virtual ctResults OnSave(ctJSONWriter& configFile);
   virtual void OnEditor();

   void SetArg(const char* name, const char* value);
   const char* GetArg(const char* name, const char* defaultValue);

protected:
   ctStringUtf8 assetFilePath;
   ctStringUtf8 fileName;

   class CT_API Output {
   public:
      inline Output() {
         nicknamepostfix = ctStringUtf8();
         guid = ctGUID();
      }
      inline Output(ctStringUtf8 newnamepostfix, ctGUID newguid) {
         nicknamepostfix = newnamepostfix;
         guid = newguid;
      }
      ctStringUtf8 nicknamepostfix; /* todo: replace output alias with nickname postfix */
      ctGUID guid;
   };

   class CT_API Arg {
   public:
      inline Arg() {
         key = "";
         value = "";
      }
      inline Arg(ctStringUtf8 newkey, ctStringUtf8 newvalue) {
         key = newkey;
         value = newvalue;
      }
      ctStringUtf8 key;
      ctStringUtf8 value;
   };

   ctStringUtf8 savedtypename;
   ctStringUtf8 nickname; /* todo: asset compiler by default it will be the file path in
                             lowercase with underscores instead of slashes */
   ctDynamicArray<Output> outputs;
   ctDynamicArray<Arg> args;

   bool DoUIArgBool(const char* name, const char* label, bool defaultValue);
   int
   DoUIArgInt(const char* name, const char* label, int defaultValue, int min, int max);
   float DoUIArgFloat(
     const char* name, const char* label, float defaultValue, float min, float max);
   int DoUIArgCombo(const char* name,
                     const char* label,
                     int defaultSlot,
                     size_t optionCount,
                     const char** options);
   void DoUIArgString(
       const char* name, const char* label, const char* defaultValue);
};