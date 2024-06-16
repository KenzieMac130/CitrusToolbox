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

#include "TextureResource.hpp"

const char* ctResourceTexture::GetName() {
   return "Texture";
}

ctResults ctResourceTexture::LoadTask() {
   return CT_SUCCESS;
}

void ctResourceTexture::OnRelease() {
}

bool ctResourceTexture::isHotReloadSupported() {
   return true;
}

void ctResourceTexture::OnReloadComplete() {
}

ctResourceBase* ctResourceServerTexture::NewResource(ctGUID guid) {
   return new ctResourceTexture(this, Engine, guid);
}
