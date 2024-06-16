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
#include "formats/mo/MO.h"

class ctResourceTranslation : public ctResourceBase {
public:
   ctResourceTranslation(ctResourceServerBase* pServer,
                         ctEngineCore* pEngine,
                         ctGUID guid) :
       ctResourceBase(pServer, pEngine, guid) {
      mo = ctMOReader();
   };
   inline const char* FindTranslation(const char* native) {
      ctAssert(isReady());
      return ctMOFindTranslation(&mo, native);
   }
   virtual const char* GetName();

protected:
   virtual ctResults LoadTask();
   virtual void OnRelease();
   virtual bool isHotReloadSupported();
   virtual void OnReloadComplete();
   ctMOReader mo;
};

class ctResourceServerTranslation : public ctResourceServerBase {
public:
   virtual ctResourceBase* NewResource(ctGUID guid);
};