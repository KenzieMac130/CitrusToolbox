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

#include "ModelInternal.hpp"

CT_API ctModel ctModelCreateEmpty() {
   return new ctModelT();
}

CT_API ctResults ctModelLoad(ctModel* pModel, void* vfile) {
   *pModel = new ctModelT();
   ctModelT& model = **pModel;
   ctFile& file = *(ctFile*)vfile;

   /* load header */
   ctModelHeader& header = model.header;
   file.ReadRaw(&header, sizeof(header), 1);

   /* load wad contents into memory */
   model.inMemoryWadDataSize = header.wadSize;
   model.inMemoryWadData = (uint8_t*)ctMalloc(header.wadSize);
   file.Seek(header.wadOffset, CT_FILE_SEEK_SET);
   file.ReadRaw(&header, header.wadSize, 1);

   /* point to each section */
   ctWADReader wad;
   CT_RETURN_FAIL(
     ctWADReaderBind(&wad, model.inMemoryWadData, model.inMemoryWadDataSize));
   // todo

   return CT_SUCCESS;
}

CT_API ctResults ctModelSave(ctModel pModel, void* vfile) {
   ctModelT& model = *pModel;
   ctFile& file = *(ctFile*)vfile;

   /* setup header section */
   ctModelHeader& header = model.header;
   file.Seek(sizeof(header), CT_FILE_SEEK_SET);
   header.wadOffset = sizeof(header);

   /* write wad sections */
   // todo

   header.wadSize = (uint64_t)file.Tell() - sizeof(header);

   /* write gpu data */
   if (model.meshData.inMemoryGeometryData) {
      header.gpuOffset = (uint64_t)file.Tell();
      header.gpuSize = model.meshData.inMemoryGeometryDataSize;
      file.WriteRaw(
        model.meshData.inMemoryGeometryData, model.meshData.inMemoryGeometryDataSize, 1);
   }
   /* write header */
   file.Seek(0, CT_FILE_SEEK_SET);
   file.WriteRaw(&header, sizeof(header), 1);
   return CT_SUCCESS;
}

CT_API const char* ctModelGetString(const ctModel model, int32_t handle) {
   return &model->stringPool[handle];
}

CT_API int32_t ctModelCreateString(ctModel model, const char* string) {
   size_t strSize = strlen(string) + 1;
   size_t originalLength = model->stringPoolSize;
   size_t desiredLength = model->stringPoolSize + strSize;
   if (model->stringDataMapped) {
      char* newBuffer = (char*)ctMalloc(desiredLength);
      memcpy(newBuffer, model->stringPool, originalLength);
      model->stringPool = newBuffer;
      model->stringDataMapped = false;
   } else {
      model->stringPool = (char*)ctRealloc(model->stringPool, desiredLength);
   }
   memcpy(&model->stringPool[originalLength], string, strSize);
   return (int32_t)originalLength;
}
