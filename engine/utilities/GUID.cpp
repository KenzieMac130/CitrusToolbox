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

#include "GUID.hpp"

#include "system/System.h"

ctGUID::ctGUID() {
   ZoneScoped;
   memset(data, 0, 16);
}

ctGUID::ctGUID(const char* hexString, size_t size) {
   memset(data, 0, sizeof(data));
   if (size != 32) { return; }
   ctHexToBytes(16, hexString, data);
}

ctGUID::ctGUID(const char* hexString) {
   memset(data, 0, sizeof(data));
   if (strlen(hexString) != 32) { return; }
   ctHexToBytes(16, hexString, data);
}

ctGUID::ctGUID(char hexString[32]) {
   ZoneScoped;
   memset(data, 0, sizeof(data));
   ctHexToBytes(16, hexString, data);
}

void ctGUID::ToHex(char dest[32]) const {
   ZoneScoped;
   ctBytesToHex(16, data, dest);
}

ctResults ctGUID::Generate() {
   ZoneScoped;
   return ctSystemCreateGUID((void*)data) == 0 ? CT_SUCCESS : CT_FAILURE_UNKNOWN;
}

bool ctGUID::isValid() const {
   const uint64_t empty[2] = {0, 0};
   return memcmp(data, empty, 16) != 0;
}

ctResults ctGUIDFromAssetPath(ctGUID& result,
                              const char* assetPath,
                              const char* output) {
   ctStringUtf8 ctacPath = assetPath;
   ctacPath += ".ctac";
   ctacPath.FilePathLocalize();
   ctFile ctac = ctFile();
   CT_RETURN_FAIL(ctac.Open(ctacPath.CStr(), CT_FILE_OPEN_READ_TEXT));
   ctStringUtf8 string;
   ctac.GetText(string);
   ctac.Close();

   ctJSONReader json = ctJSONReader();
   CT_RETURN_FAIL(json.BuildJsonForPtr(string.CStr(), string.ByteLength()));

   ctJSONReadEntry root;
   CT_RETURN_FAIL(json.GetRootEntry(root));

   ctJSONReadEntry guids;
   CT_RETURN_FAIL(root.GetObjectEntry("guids", guids));

   ctJSONReadEntry guid;
   CT_RETURN_FAIL(root.GetObjectEntry(output, guid));

   ctStringUtf8 str;
   guid.GetString(str);

   result = ctGUID(str.CStr());
   return CT_SUCCESS;
}