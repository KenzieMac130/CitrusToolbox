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

class CT_API ctAnimMorphSet {
public:
   ctAnimMorphSet(const ctAnimMorphSet& base);
   ctAnimMorphSet(const struct ctModel& model);
   ~ctAnimMorphSet();

   inline int32_t GetTargetCount() const {
      return morphCount;
   }
   void SetTarget(int32_t index, float value);
   void SetTarget(const char* name, float value);
   float GetTarget(int32_t idx) const;
   float GetTarget(const char* name) const;
   void GetTargetName(int32_t idx, char output[32]) const;

protected:
   ctAnimMorphSet(uint32_t morphCount);
   friend class ctAnimCanvas;
   int32_t morphCount;
   struct ctModelMeshMorphTargetMapping* pMappings;
};