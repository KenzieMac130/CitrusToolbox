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

#include "KinnowHeaderParser.h"

#define STB_C_LEXER_IMPLEMENTATION
#include "stb/stb_c_lexer.h"

void* ctKinnowReflectArenaReserve(ctKinnowReflectParseContext& ctx, size_t count) {
   void* result = &ctx.pArena[ctx.arenaNext];
   if (ctx.arenaNext + count > ctx.arenaCapacity) { return NULL; }
   ctx.arenaNext += count;
   return result;
}

void* ctKinnowReflectArenaAddBytes(ctKinnowReflectParseContext& ctx,
                                   uint8_t* pBytes,
                                   size_t count) {
   void* result = &ctx.pArena[ctx.arenaNext];
   if (ctx.arenaNext + count > ctx.arenaCapacity) { return NULL; }
   memcpy(&ctx.pArena[ctx.arenaNext], pBytes, count);
   ctx.arenaNext += count;
}

void* ctKinnowReflectArenaAddByte(ctKinnowReflectParseContext& ctx, uint8_t byte) {
   void* result = &ctx.pArena[ctx.arenaNext];
   if (ctx.arenaNext + 1 > ctx.arenaCapacity) { return NULL; }
   ctx.pArena[ctx.arenaNext] = byte;
   ctx.arenaNext++;
}

const char* ctKinnowReflectArenaAddString(ctKinnowReflectParseContext& ctx,
                                          const char* str,
                                          size_t length) {
   const char* result = (const char*)&ctx.pArena[ctx.arenaNext];
   ctKinnowReflectArenaAddBytes(ctx, (uint8_t*)str, length);
   ctKinnowReflectArenaAddByte(ctx, 0);
   return result;
}

#define CT_KINNOW_REFLECT_ARENA_ALLOC(_ctx, _type, _count)                               \
   ctKinnowReflectArenaReserve(_ctx, sizeof(_type) * _count);

ctResults ctKinnowReflectParseCHeader(ctKinnowReflectParseContext* pCtx,
                                      const char* header,
                                      size_t length,
                                      size_t capacity) {
   /* Initialize context */
   if (pCtx->pArena) { ctKinnowReflectParseContextFree(pCtx); }
   *pCtx = ctKinnowReflectParseContext();
   pCtx->arenaCapacity = capacity;
   pCtx->pArena = (uint8_t*)ctMalloc(capacity);
   memset(pCtx->pArena, 0, pCtx->arenaCapacity);

   /* Initialize parser */
   char* strpool = (char*)ctMalloc(length + 1);
   stb_lexer lexer;
   stb_c_lexer_init(&lexer, header, header + length, strpool, length);

   /* Initialize base data */
   ctKinnowReflectHeader* pHeader = (ctKinnowReflectHeader*)CT_KINNOW_REFLECT_ARENA_ALLOC(
     *pCtx, ctKinnowReflectHeader, 1);

   ctKinnowReflectEnumField* pEnums =
     (ctKinnowReflectEnumField*)CT_KINNOW_REFLECT_ARENA_ALLOC(
       *pCtx, ctKinnowReflectEnumField, CT_MAX_KINNOW_REFLECT_ENUMS);
   pHeader->pEnums = pEnums;

   ctKinnowReflectStructField* pStructs =
     (ctKinnowReflectStructField*)CT_KINNOW_REFLECT_ARENA_ALLOC(
       *pCtx, ctKinnowReflectStructField, CT_MAX_KINNOW_REFLECT_MAX_STRUCTS);
   pHeader->pStructs = pStructs;

   ctFree(strpool);
}

ctResults ctKinnowReflectParseSerialize(ctKinnowReflectParseContext* pCtx,
                                        uint8_t* pDest,
                                        size_t capacity) {
   /* Copy to dest and normalize pointers */
   return CT_SUCCESS;
}

ctResults ctKinnowReflectParseDeserialize(ctKinnowReflectParseContext* pCtx,
                                          uint8_t* pSrc,
                                          size_t capacity) {
   /* Copy to arena and denormalize pointers */
   return CT_SUCCESS;
}

ctResults ctKinnowReflectParseContextFree(ctKinnowReflectParseContext* pCtx) {
   ctFree(pCtx->pArena);
   *pCtx = ctKinnowReflectParseContext();
   return CT_SUCCESS;
}
