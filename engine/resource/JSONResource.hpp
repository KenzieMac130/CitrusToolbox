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
#include "ResourceTypeBase.hpp"
#include "formats/json/JSON.hpp"

class ctResourceJSON : public ctResourceBase {
public:
   ctResourceJSON(ctResourceServerBase* pServer, ctEngineCore* pEngine, ctGUID guid) :
       ctResourceBase(pServer, pEngine, guid) {
      reader = ctJSONReader();
   };
   inline ctResults GetRootEntry(ctJSONReadEntry& entry) {
      return reader.GetRootEntry(entry);
   }

   virtual const char* GetName();

protected:
   virtual ctResults LoadTask();
   virtual void OnRelease();

   ctStringUtf8 string;
   ctJSONReader reader;
};

class ctResourceServerJSON : public ctResourceServerBase {
public:
   virtual ctResourceBase* NewResource(ctGUID guid);
};