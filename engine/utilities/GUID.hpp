/*
   Copyright 2022 MacKenzie Strand

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

#include "Common.h"

class ctGUID {
public:
   ctGUID();
   ctGUID(const char* hexString, size_t size);
   ctGUID(const char* hexString);
   ctGUID(char hexString[32]);

   ctResults Generate();
   bool isValid() const;
   void ToHex(char dest[32]) const;
   inline bool operator==(const ctGUID& other) const {
      return memcmp(data, other.data, 16) == 0;
   }
   uint8_t data[16];
};

CT_API ctResults ctGUIDFromAssetPath(ctGUID& result, const char* assetPath, const char* output = "OUTPUT");