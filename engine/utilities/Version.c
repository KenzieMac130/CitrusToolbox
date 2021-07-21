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

#include "engine_build_dummy.h"

#include "xxhash/xxhash.h"

unsigned int ctGetEngineBuildId() {
   const char* timestamp = __TIMESTAMP__;
   size_t size = 0;
   for (const char* p = timestamp; *p != 0; p++) {
      size++;
   }
   return XXH32(timestamp, size, 0);
}