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

#include "BinaryResource.hpp"

const char* ctResourceBinary::GetName() {
   return "Text";
}

ctResults ctResourceBinary::LoadTask() {
   ctFile file;
   CT_RETURN_FAIL(Engine->FileSystem->OpenDataFileByGUID(file, GetDataGUID()));
   file.GetBytes(bytes);
   file.Close();
   return CT_SUCCESS;
}

void ctResourceBinary::OnRelease() {
   /* destructor handles this */
}

ctResourceBase* ctResourceServerBinary::NewResource(ctGUID guid) {
   return new ctResourceBinary(this, Engine, guid);
}