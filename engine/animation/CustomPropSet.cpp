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

#include "MorphSet.hpp"
#include "CustomPropSet.hpp"

ctAnimCustomPropSet::ctAnimCustomPropSet(uint32_t count,
                                         const char** propNames,
                                         float* defaultValues) {
   propCount = count;
   pProps = (ctAnimCustomProp*)ctMalloc(sizeof(pProps[0]) * propCount);
   memset(pProps, 0, sizeof(pProps[0]) * propCount);
   for (int32_t i = 0; i < propCount; i++) {
      if (propNames) { strncpy(pProps[i].name, propNames[i], 32); }
      if (defaultValues) { pProps[i].value = defaultValues[i]; }
   }
}

ctAnimCustomPropSet::ctAnimCustomPropSet(const ctAnimCustomPropSet& base) {
   propCount = base.propCount;
   pProps = (ctAnimCustomProp*)ctMalloc(sizeof(pProps[0]) * propCount);
   memcpy(pProps, base.pProps, sizeof(pProps[0]) * propCount);
}

ctAnimCustomPropSet::~ctAnimCustomPropSet() {
   ctFree(pProps);
}

void ctAnimCustomPropSet::SetProp(const char* name, float value) {
   for (int32_t i = 0; i < propCount; i++) {
      if (ctCStrNEql(name, pProps[i].name, 32)) { SetProp(i, value); }
   }
}

void ctAnimCustomPropSet::SetProp(int32_t idx, float value) {
   pProps[idx].value = value;
}

float ctAnimCustomPropSet::GetProp(int32_t idx) const {
   return pProps[idx].value;
}

float ctAnimCustomPropSet::GetProp(const char* name) const {
   for (int32_t i = 0; i < propCount; i++) {
      if (ctCStrNEql(name, pProps[i].name, 32)) { return GetProp(i); }
   }
   return 0.0f;
}

void ctAnimCustomPropSet::GetPropName(int32_t idx, char output[32]) const {
   memcpy(output, pProps[idx].name, 32);
}
