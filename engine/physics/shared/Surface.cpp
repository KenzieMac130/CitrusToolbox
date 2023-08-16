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

#include "Surface.hpp"

ctPhysicsSurfaceTypeT::ctPhysicsSurfaceTypeT(ctJSONReadEntry dict, const char* inName) :
    ctPhysicsSurfaceTypeT() {
   name = inName;
   for (size_t i = 0; i < dict.GetObjectEntryCount(); i++) {
      ctJSONReadEntry varentry;
      ctStringUtf8 varname;
      if (dict.GetObjectEntry(i, varentry, &varname) != CT_SUCCESS) { continue; }

      if (varname == "parent") {
         ctStringUtf8 value = "";
         varentry.GetString(value);
         parentHash = value.xxHash32();
      } else if (varentry.isBool()) {
         bool value = false;
         varentry.GetBool(value);
         flags.Insert(varname.xxHash32(), value);
      } else if (varentry.isNumber()) {
         float value = false;
         varentry.GetNumber(value);
         scalars.Insert(varname.xxHash32(), value);
      } else {
         ctStringUtf8 value = "";
         varentry.GetString(value);
         strings.Insert(varname.xxHash32(), value);
      }
   }
}

ctPhysicsSurfaceTypeFactory::ctPhysicsSurfaceTypeFactory(ctJSONReadEntry jsonRoot) {
   for (size_t i = 0; i < jsonRoot.GetObjectEntryCount(); i++) {
      ctJSONReadEntry varentry;
      ctStringUtf8 varname;
      if (jsonRoot.GetObjectEntry(i, varentry, &varname) != CT_SUCCESS) { continue; }
      ctPhysicsSurfaceTypeT* surface =
        new ctPhysicsSurfaceTypeT(varentry, varname.CStr());
      surfaces.Insert(varname.xxHash32(), surface);
   }
}

ctPhysicsSurfaceTypeFactory::~ctPhysicsSurfaceTypeFactory() {
   for (auto it = surfaces.GetIterator(); it; it++) {
      delete it.Value();
   }
}

void ctPhysicsSurfaceTypeFactory::BuildParents() {
   for (auto it = surfaces.GetIterator(); it; it++) {
      if (it.Value()->parentHash != 0) {
         ctPhysicsSurfaceTypeT** surf = surfaces.FindPtr(it.Value()->parentHash);
         if (surf) { it.Value()->parent = *surf; }
      }
   }
}
