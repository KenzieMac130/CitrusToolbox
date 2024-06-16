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

#include "ShaderResource.hpp"

const char* ctResourceShader::GetName() {
   return "Shader";
}

ctResults ctResourceShader::LoadTask() {
   ctFile file;
   CT_RETURN_FAIL(Engine->FileSystem->OpenDataFileByGUID(file, GetDataGUID()));
   file.GetBytes(bytes);
   file.Close();
   CT_RETURN_FAIL(ctWADReaderBind(&wad, bytes.Data(), bytes.Count()));
   return CT_SUCCESS;
}

void ctResourceShader::OnRelease() {
}

bool ctResourceShader::isHotReloadSupported() {
   return false;
}

void ctResourceShader::OnReloadComplete() {
}

ctResourceBase* ctResourceServerShader::NewResource(ctGUID guid) {
   return new ctResourceShader(this, Engine, guid);
}
