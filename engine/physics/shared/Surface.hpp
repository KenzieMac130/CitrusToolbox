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

class ctPhysicsSurfaceTypeT {
public:
   ctPhysicsSurfaceTypeT() = default;
   ctPhysicsSurfaceTypeT(ctJSONReadEntry dict, const char* name);

   inline const char* GetName() {
      return name.CStr();
   }

   inline float* GetScalar(uint32_t hash) {
      float* result = scalars.FindPtr(hash);
      if (!result && parent) {
         result = parent->GetScalar(hash);
         if (!result) { return NULL; }
      }
      return result;
   }
   inline const char* GetString(uint32_t hash) {
      ctStringUtf8* pstr = strings.FindPtr(hash);
      const char* result = pstr ? pstr->CStr() : NULL;
      if (!result && parent) {
         result = parent->GetString(hash);
         if (!result) { return NULL; }
      }
      return result;
   }
   inline bool* GetFlag(uint32_t hash) {
      bool* result = flags.FindPtr(hash);
      if (!result && parent) {
         result = parent->GetFlag(hash);
         if (!result) { return NULL; }
      }
      return result;
   }

protected:
   friend class ctPhysicsSurfaceTypeFactory;
   ctStringUtf8 name;
   ctHashTable<float, uint32_t> scalars;
   ctHashTable<ctStringUtf8, uint32_t> strings;
   ctHashTable<bool, uint32_t> flags;
   uint32_t parentHash;
   ctPhysicsSurfaceTypeT* parent;
};

class ctPhysicsSurfaceTypeFactory {
public:
   ctPhysicsSurfaceTypeFactory(ctJSONReadEntry jsonRoot);
   ~ctPhysicsSurfaceTypeFactory();

   inline ctHashTable<ctPhysicsSurfaceTypeT*, uint32_t>& GetSurfaceMap() {
      return surfaces;
   }

private:
   void BuildParents();

   ctHashTable<ctPhysicsSurfaceTypeT*, uint32_t> surfaces;
};