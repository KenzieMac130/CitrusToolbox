#pragma once

#include "xxhash/xxhash.h"

uint32_t ctxxHash32(const void* pData, const size_t size, int seed);
uint32_t ctxxHash32(const char* pStr, int seed);
uint32_t ctxxHash32(const void* pData, const size_t size);
uint32_t ctxxHash32(const char* pStr);

uint64_t ctxxHash64(const void* pData, const size_t size, int seed);
uint64_t ctxxHash64(const char* pStr, int seed);
uint64_t ctxxHash64(const void* pData, const size_t size);
uint64_t ctxxHash64(const char* pStr);