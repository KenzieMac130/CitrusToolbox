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

#include "Image.hpp"

ctResults ctImage::CompileOffline(const ctStringUtf8& filePath,
                                  enum TinyImageFormat outFormat,
                                  uint32_t outMipCount) {
   /* save current to temp folder */
   ctFile tmpFile = ctFile();
   CT_RETURN_FAIL(tmpFile.OpenTemp(".png"));
   CT_RETURN_FAIL(Save(tmpFile));
   tmpFile.Close();

   /* call compressonator */
   ctStringUtf8 mipCountString = "";
   mipCountString.Printf(8, "%u", outMipCount);
   ctStringUtf8 formatString = "";
   /* todo */
   const char* args[6];
   args[0] = 
   ctSystemExecuteCommand(CT_COMPRESSONATOR_PATH, 
}
