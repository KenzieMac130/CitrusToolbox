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

#include "File.hpp"

const char* modestr[] = {"rb", "wb", "r", "w"};

ctFile::ctFile() {
   _fSize = -1;
   _ctx = NULL;
   _mode = CT_FILE_OPEN_READ;
}

ctFile::ctFile(FILE* fp, const ctFileOpenMode mode) : ctFile() {
   FromCStream(fp, mode);
};

ctFile::ctFile(void* memory, size_t size, const ctFileOpenMode mode) {
   _fSize = -1;
   if (mode == CT_FILE_OPEN_WRITE || mode == CT_FILE_OPEN_WRITE_TEXT) {
      _ctx = SDL_RWFromMem(memory, (int)size);
   } else {
      _ctx = SDL_RWFromConstMem(memory, (int)size);
   }
   _mode = mode;
}

ctFile::ctFile(const void* memory, size_t size, const ctFileOpenMode mode) {
   _fSize = -1;
   _ctx = SDL_RWFromConstMem(memory, (int)size);
   _mode = mode;
}

ctFile::ctFile(const ctStringUtf8& filePath, const ctFileOpenMode mode, bool silent) :
    ctFile() {
   Open(filePath, mode, silent);
}

ctFile::~ctFile() {
   if (isOpen()) { Close(); }
}

void ctFile::FromCStream(FILE* fp, const ctFileOpenMode mode, bool allowClose) {
   ZoneScoped;
   if (isOpen()) { Close(); }
   _ctx = SDL_RWFromFP(fp, allowClose ? SDL_TRUE : SDL_FALSE);
   _mode = mode;
}

ctResults
ctFile::Open(const ctStringUtf8& filePath, const ctFileOpenMode mode, bool silent) {
   ZoneScoped;
   if (mode > 3 || mode < 0) { return CT_FAILURE_INVALID_PARAMETER; }
   _ctx = SDL_RWFromFile(filePath.CStr(), modestr[mode]);
   _mode = mode;
   if (_ctx) {
      return CT_SUCCESS;
   } else {
      if (!silent) {
         ctDebugError("COULD NOT OPEN: %s - %s", filePath.CStr(), modestr[mode])
      };
      return CT_FAILURE_INACCESSIBLE;
   }
}

void ctFile::Close() {
   ZoneScoped;
   if (_ctx) { SDL_RWclose(_ctx); }
   _ctx = NULL;
}

int64_t ctFile::GetFileSize() {
   ZoneScoped;
   if (!_ctx) { return 0; }
   if (_fSize == -1) { _fSize = SDL_RWsize(_ctx); }
   if (_fSize == -1) {
      size_t cur = Tell();
      Seek(0, CT_FILE_SEEK_END);
      _fSize = Tell();
      Seek(cur, CT_FILE_SEEK_SET);
   }
   return _fSize;
}

size_t ctFile::GetBytes(uint8_t** ppOutBytes) {
   ZoneScoped;
   ctAssert(ppOutBytes);
   const size_t fsize = (size_t)GetFileSize();
   *ppOutBytes = (uint8_t*)ctMalloc(fsize);
   ReadRaw(*ppOutBytes, 1, fsize);
   return fsize;
}

size_t ctFile::GetBytes(ctDynamicArray<uint8_t>& outArray) {
   ZoneScoped;
   const size_t fsize = (size_t)GetFileSize();
   outArray.Resize(fsize);
   ReadRaw(outArray.Data(), 1, outArray.Count());
   return fsize;
}

size_t ctFile::GetBytes(ctDynamicArray<char>& outArray) {
   ZoneScoped;
   const size_t fsize = (size_t)GetFileSize();
   outArray.Resize(fsize);
   ReadRaw(outArray.Data(), 1, outArray.Count());
   return fsize;
}

size_t ctFile::GetText(ctStringUtf8& outString) {
   ZoneScoped;
   const size_t fsize = (size_t)GetFileSize();
   outString = "";
   outString.Append('\0', fsize + 1);
   ReadRaw(outString.Data(), 1, fsize);
   return fsize;
}

int64_t ctFile::Tell() {
   ZoneScoped;
   if (!_ctx) { return 0; }
   return SDL_RWtell(_ctx);
}

ctResults ctFile::Seek(const int64_t offset, const ctFileSeekMode mode) {
   ZoneScoped;
   if (!_ctx) { return CT_FAILURE_INACCESSIBLE; }
   return SDL_RWseek(_ctx, offset, mode) == 0 ? CT_SUCCESS : CT_FAILURE_INACCESSIBLE;
}

size_t ctFile::ReadRaw(void* pDest, const size_t size, const size_t count) {
   ZoneScoped;
   if (!_ctx) { return 0; }
   return SDL_RWread(_ctx, pDest, size, count);
}

size_t ctFile::WriteRaw(const void* pData, size_t size, const size_t count) {
   ZoneScoped;
   if (!_ctx) { return 0; }
   return SDL_RWwrite(_ctx, pData, size, count);
}

size_t ctFile::Printf(const char* format, ...) {
   ZoneScoped;
   va_list args;
   va_start(args, format);
   const size_t result = VPrintf(format, args);
   va_end(args);
   return result;
}

size_t ctFile::VPrintf(const char* format, va_list va) {
   ZoneScoped;
   if (!_ctx) { return 0; }
   va_list vaCpy;
   va_copy(vaCpy, va);
   int buffLen = vsnprintf(NULL, 0, format, vaCpy) + 1;
   if (buffLen == 0) { return 0; }
   char* buffer = NULL;
   bool isHeap = buffLen > 4096;
   if (isHeap) {
      buffer = (char*)ctMalloc(buffLen);
   } else {
      buffer = (char*)ctStackAlloc(buffLen);
   }
   memset(buffer, 0, buffLen);
   vsnprintf(buffer, buffLen, format, va);
   size_t stringLength = strlen(buffer);
   size_t result = WriteRaw(buffer, 1, stringLength);
   if (isHeap) { ctFree(buffer); }
   return result;
}

#if defined(__WIN32__)
#include <Windows.h>
#endif

void ctFile::Flush() {
   /* Bending the rules a little bit and messing with SDL guts */
   if (!_ctx) { return; }
#if defined(__WIN32__)
   if (_ctx->type == SDL_RWOPS_WINFILE) {
      if (!_ctx->hidden.windowsio.h) { return; }
      FlushFileBuffers(_ctx->hidden.windowsio.h);
   }
#endif
#ifdef HAVE_STDIO_H
   if (_ctx->type == SDL_RWOPS_STDFILE) {
      if (!_ctx->hidden.stdio.fp) { return; }
      fflush(_ctx->hidden.stdio.fp);
   }
#endif
}

bool ctFile::isEndOfFile() {
   const size_t fileSize = GetFileSize();
   return (size_t)Tell() >= fileSize;
}

bool ctFile::isOpen() const {
   return _ctx;
}