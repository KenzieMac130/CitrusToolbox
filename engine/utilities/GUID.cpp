/*
   Copyright 2021 MacKenzie Strand

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

#include "GUID.hpp"

#include "system/System.h"

ctGUID::ctGUID() {
   ZoneScoped;
   ctAssert(ctSystemCreateGUID((void*)data) == 0);
}

ctGUID::ctGUID(const char* hexString) {
   memset(data, 0, sizeof(data));
   if (strlen(hexString) != 32) { return; }
   memset(data, 0, sizeof(data));
   ctHexToBytes(16, hexString, data);
}

ctGUID::ctGUID(char hexString[32]) {
   ZoneScoped;
   memset(data, 0, sizeof(data));
   ctHexToBytes(16, hexString, data);
}

void ctGUID::ToHex(char dest[32]) const {
   ctBytesToHex(16, data, dest);
}
