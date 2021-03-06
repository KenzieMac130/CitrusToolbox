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

#pragma once

#include "Common.h"
#include "xxhash/xxhash.h"

CT_API uint32_t ctxxHash32(const void* pData, const size_t size, int seed);
CT_API uint32_t ctxxHash32(const char* pStr, int seed);
CT_API uint32_t ctxxHash32(const void* pData, const size_t size);
CT_API uint32_t ctxxHash32(const char* pStr);

CT_API uint64_t ctxxHash64(const void* pData, const size_t size, int seed);
CT_API uint64_t ctxxHash64(const char* pStr, int seed);
CT_API uint64_t ctxxHash64(const void* pData, const size_t size);
CT_API uint64_t ctxxHash64(const char* pStr);