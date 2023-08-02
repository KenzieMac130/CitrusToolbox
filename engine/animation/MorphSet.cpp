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
#include "formats/model/Model.hpp"

ctAnimMorphSet::ctAnimMorphSet(uint32_t count) {
   morphCount = count;
   pMappings =
     (ctModelMeshMorphTargetMapping*)ctMalloc(sizeof(pMappings[0]) * morphCount);
}

ctAnimMorphSet::ctAnimMorphSet(const ctAnimMorphSet& base) :
    ctAnimMorphSet(base.morphCount) {
}

ctAnimMorphSet::ctAnimMorphSet(const ctModel& model) :
    ctAnimMorphSet(model.geometry.morphTargetMappingCount) {
   memcpy(pMappings,
          model.geometry.morphTargetMapping,
          sizeof(ctModelMeshMorphTargetMapping) * morphCount);
}

ctAnimMorphSet::~ctAnimMorphSet() {
   ctFree(pMappings);
}

void ctAnimMorphSet::SetTarget(int32_t index, float value) {
   pMappings[index].value = value;
}

void ctAnimMorphSet::SetTarget(const char* name, float value) {
   for (int32_t i = 0; i < morphCount; i++) {
      if (ctCStrNEql(name, pMappings[i].name, 32)) { SetTarget(i, value); }
   }
}

float ctAnimMorphSet::GetTarget(int32_t idx) const {
   return pMappings[idx].value;
}

float ctAnimMorphSet::GetTarget(const char* name) const {
   for (int32_t i = 0; i < morphCount; i++) {
      if (ctCStrNEql(name, pMappings[i].name, 32)) { return GetTarget(i); }
   }
   return 0.0f;
}

void ctAnimMorphSet::GetTargetName(int32_t idx, char output[32]) const {
   memcpy(output, pMappings[idx].name, 32);
}
