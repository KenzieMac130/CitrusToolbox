#define XXH_STATIC_LINKING_ONLY
#define XXH_IMPLEMENTATION
#include "Hash.hpp"

uint32_t ctxxHash32(const void* pData, const size_t size, int seed) {
   return XXH32(pData, size, seed);
}

uint32_t ctxxHash32(const char* pStr, int seed) {
   return ctxxHash32(pStr, strlen(pStr), seed);
}

uint32_t ctxxHash32(const void* pData, const size_t size) {
   return ctxxHash32(pData, size, 0);
}

uint32_t ctxxHash32(const char* pStr) {
   return ctxxHash32(pStr, 0);
}

uint64_t ctxxHash64(const void* pData, const size_t size, int seed) {
   return XXH64(pData, size, seed);
}

uint64_t ctxxHash64(const char* pStr, int seed) {
   return ctxxHash64(pStr, strlen(pStr), seed);
}

uint64_t ctxxHash64(const void* pData, const size_t size) {
   return ctxxHash64(pData, size, 0);
}

uint64_t ctxxHash64(const char* pStr) {
   return ctxxHash64(pStr, strlen(pStr), 0);
}