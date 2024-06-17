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

#include "JSONResource.hpp"

const char* ctResourceJSON::GetName() {
   return "JSON";
}

ctResults ctResourceJSON::LoadTask() {
   ctFile file;
   CT_RETURN_FAIL(Engine->FileSystem->OpenDataFileByGUID(file, GetDataGUID()));
   file.GetText(string);
   file.Close();
   CT_RETURN_FAIL(reader.BuildJsonForPtr(string.CStr(), string.ByteLength()));
   CT_RETURN_FAIL(reader.GetRootEntry(ctJSONReadEntry())); /* check parse */
   return CT_SUCCESS;
}

void ctResourceJSON::OnRelease() {
   /* destructor handles this */
}

ctResourceBase* ctResourceServerJSON::NewResource(ctGUID guid) {
   return new ctResourceJSON(this, Engine, guid);
}
