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

#include "TranslationResource.hpp"

const char* ctResourceTranslation::GetName() {
   return "Translation";
}

ctResults ctResourceTranslation::LoadTask() {
   ctFile file;
   CT_RETURN_FAIL(Engine->FileSystem->OpenDataFileByGUID(file, GetDataGUID()));
   ctDynamicArray<uint8_t> bytes;
   file.GetBytes(bytes);
   file.Close();
   CT_RETURN_FAIL(ctMOReaderInitialize(&mo, bytes.Data(), bytes.Count()));
   return CT_SUCCESS;
}

void ctResourceTranslation::OnRelease() {
   ctMOReaderRelease(&mo);
}

bool ctResourceTranslation::isHotReloadSupported() {
   return true;
}

void ctResourceTranslation::OnReloadComplete() {
   /* todo: signal to the translation system to reload */
}

ctResourceBase* ctResourceServerTranslation::NewResource(ctGUID guid) {
   return new ctResourceTranslation(this, Engine, guid);
}