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

struct ctAnimCustomProp {
    char name[32];
    float value;
};

class CT_API ctAnimCustomPropSet {
public:
   ctAnimCustomPropSet(uint32_t propCount,
                       const char** propNames = NULL,
                       float* defaultValues = NULL);
   ctAnimCustomPropSet(const ctAnimCustomPropSet& base);
   ~ctAnimCustomPropSet();

   inline int32_t GetPropCount() const {
      return propCount;
   }
   void SetProp(const char* name, float value);
   void SetProp(int32_t idx, float value);
   float GetProp(int32_t idx) const;
   float GetProp(const char* name) const;
   void GetPropName(int32_t idx, char output[32]) const;

protected:
   friend class ctAnimCanvas;
   int32_t propCount;
   struct ctAnimCustomProp* pProps;
};