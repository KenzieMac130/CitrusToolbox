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

/* https://xueyouchao.github.io/2016/11/16/CompileTimeString/ */
template<uint64_t N>
constexpr inline uint64_t
CT_COMPILE_HORNER_HASH_CONSTEXPR(uint64_t prime, const char (&str)[N], uint64_t Len = N - 1) {
   return (Len <= 1) ? str[0]
                     : (prime * CT_COMPILE_HORNER_HASH_CONSTEXPR(prime, str, Len - 1) +
                        str[Len - 1]);
}
#define CT_COMPILE_HORNER_HASH(_STR) (CT_COMPILE_HORNER_HASH_CONSTEXPR(31, _STR))

inline uint64_t ctHornerHash(const char* pStr, uint64_t prime) {
    if (pStr == NULL) return 0;
    uint64_t hash = *pStr;
    for (; *(pStr + 1) != '\0'; pStr++) {
        hash = prime * hash + *(pStr + 1);
    }
    return hash;
}
inline uint64_t ctHornerHash(const char* pStr) {
   return ctHornerHash(pStr, 31);
};

CT_API uint32_t ctXXHash32(const void* pData, const size_t size, uint32_t seed);
CT_API uint32_t ctXXHash32(const char* pStr, int seed);
CT_API uint32_t ctXXHash32(const void* pData, const size_t size);
CT_API uint32_t ctXXHash32(const char* pStr);

CT_API uint64_t ctXXHash64(const void* pData, const size_t size, uint64_t seed);
CT_API uint64_t ctXXHash64(const char* pStr, int seed);
CT_API uint64_t ctXXHash64(const void* pData, const size_t size);
CT_API uint64_t ctXXHash64(const char* pStr);