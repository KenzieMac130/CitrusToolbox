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

#include "WAD.hpp"

ctWADAsset::ctWADAsset(class ctAssetManager* _manager, const char* _relativePath) :
    ctAssetBase(_manager, _relativePath) {
   blob = NULL;
   blobSize = 0;
   wadReader = ctWADReader();
}

const char* ctWADAsset::GetAssetType() {
   return "wad";
}

bool ctWADAsset::isAvailible() {
   return blob;
}

ctResults ctWADAsset::OnLoad(ctFile& file) {
   blobSize = file.GetFileSize();
   blob = (uint8_t*)ctMalloc(blobSize);
   file.ReadRaw(blob, blobSize, 1);
   ctWADReaderBind(&wadReader, blob, blobSize);
   return CT_SUCCESS;
}

ctResults ctWADAsset::OnRelease() {
   ctFree(blob);
   blob = NULL;
   blobSize = 0;
   wadReader = ctWADReader();
   return CT_SUCCESS;
}
