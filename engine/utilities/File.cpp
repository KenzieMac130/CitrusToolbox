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
   _fp = NULL;
   _mode = CT_FILE_OPEN_READ;
}

ctFile::ctFile(FILE* fp, const ctFileOpenMode mode) : ctFile() {
   FromCStream(fp, mode);
};

ctFile::ctFile(const ctStringUtf8& filePath, const ctFileOpenMode mode, bool silent) :
    ctFile() {
   Open(filePath, mode, silent);
}

ctFile::~ctFile() {
   if (isOpen()) { Close(); }
}

void ctFile::FromCStream(FILE* fp, const ctFileOpenMode mode) {
   ZoneScoped;
   if (isOpen()) { Close(); }
   _fp = fp;
   _mode = mode;
}

ctResults
ctFile::Open(const ctStringUtf8& filePath, const ctFileOpenMode mode, bool silent) {
   ZoneScoped;
   if (mode >= 3 || mode < 0) { return CT_FAILURE_INVALID_PARAMETER; }
   _fp = fopen(filePath.CStr(), modestr[mode]);
   _mode = mode;
   if (_fp) {
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
   if (_fp) { fclose(_fp); }
   _fp = NULL;
}

int64_t ctFile::GetFileSize() {
   ZoneScoped;
   if (!_fp) { return 0; }
   if (_fSize == -1) {
      Seek(0, CT_FILE_SEEK_END);
      _fSize = Tell();
      Seek(0, CT_FILE_SEEK_SET);
   }
   return _fSize;
}

void ctFile::GetBytes(ctDynamicArray<uint8_t>& outArray) {
   ZoneScoped;
   outArray.Resize((size_t)GetFileSize());
   ReadRaw(outArray.Data(), 1, outArray.Count());
}

void ctFile::GetBytes(ctDynamicArray<char>& outArray) {
   ZoneScoped;
   outArray.Resize((size_t)GetFileSize());
   ReadRaw(outArray.Data(), 1, outArray.Count());
}

void ctFile::GetText(ctStringUtf8& outString) {
   ZoneScoped;
   const size_t fsize = (size_t)GetFileSize();
   outString = "";
   outString.Append('\0', fsize + 1);
   ReadRaw(outString.Data(), 1, fsize);
}

int64_t ctFile::Tell() {
   ZoneScoped;
   if (!_fp) { return 0; }
   return ftell(_fp);
}

ctResults ctFile::Seek(const int64_t offset, const ctFileSeekMode mode) {
   ZoneScoped;
   if (!_fp) { return CT_FAILURE_INACCESSIBLE; }
   return fseek(_fp, (long)offset, mode) == 0 ? CT_SUCCESS : CT_FAILURE_INACCESSIBLE;
}

size_t ctFile::ReadRaw(void* pDest, const size_t size, const size_t count) {
   ZoneScoped;
   if (!_fp) { return 0; }
   return fread(pDest, size, count, _fp);
}

size_t ctFile::WriteRaw(const void* pData, size_t size, const size_t count) {
   ZoneScoped;
   if (!_fp) { return 0; }
   return fwrite(pData, size, count, _fp);
}

int64_t ctFile::Printf(const char* format, ...) {
   ZoneScoped;
   va_list args;
   va_start(args, format);
   const int64_t result = VPrintf(format, args);
   va_end(args);
   return result;
}

int64_t ctFile::VPrintf(const char* format, va_list va) {
   ZoneScoped;
   if (!_fp) { return 0; }
   return vfprintf(_fp, format, va);
}

void ctFile::Flush() {
   fflush(_fp);
}

bool ctFile::isEndOfFile() {
   return feof(_fp);
}

FILE* ctFile::CFile() const {
   return _fp;
}

bool ctFile::isOpen() const {
   return _fp;
}