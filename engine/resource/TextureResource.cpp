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
#include "formats/image/Image.hpp"

const char* ctResourceTexture::GetName() {
   return "Texture";
}

ctResults ctResourceTexture::LoadTask() {
   ctFile file;
   CT_RETURN_FAIL(Engine->FileSystem->OpenDataFileByGUID(file, GetDataGUID()));
   ctImage image = ctImage();
   CT_RETURN_FAIL(image.Load(file));
   /* todo: create renderer texture */
   return CT_SUCCESS;
}

void ctResourceTexture::OnRelease() {
}

void ctResourceTexture::OnReloadComplete() {
   /* todo: reload texture */
}

ctResourceBase* ctResourceServerTexture::NewResource(ctGUID guid) {
   return new ctResourceTexture(this, Engine, guid);
}
