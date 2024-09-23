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

ctResults ctImage::Save(ctFile& file, const char* format) {
   if (ctCStrEql(format, "png")) {
   } else if (ctCStrEql(format, "jpg")) {
   } else if (ctCStrEql(format, "tga")) {
   } else if (ctCStrEql(format, "bmp")) {
   } else if (ctCStrEql(format, "bmp")) {
   } else if (ctCStrEql(format, "ktx")) {
   } else if (ctCStrEql(format, "dds")) {
   }
   return CT_FAILURE_UNKNOWN_FORMAT;
}
