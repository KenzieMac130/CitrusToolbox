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

#include "utilities/Common.h"

enum ctFileSeekMode {
   CT_FILE_SEEK_SET = SEEK_SET,
   CT_FILE_SEEK_CUR = SEEK_CUR,
   CT_FILE_SEEK_END = SEEK_END
};

enum ctFileOpenMode {
   CT_FILE_OPEN_READ = 0,
   CT_FILE_OPEN_WRITE = 1,
   CT_FILE_OPEN_READ_TEXT = 2,
   CT_FILE_OPEN_WRITE_TEXT = 3
};

class CT_API ctFile {
public:
   ctFile();
   ctFile(FILE* fp, const ctFileOpenMode mode);
   ctFile(const ctStringUtf8& filePath, const ctFileOpenMode mode, bool silent = false);
   ~ctFile();
   void FromCStream(FILE* fp, const ctFileOpenMode mode);
   ctResults
   Open(const ctStringUtf8& filePath, const ctFileOpenMode mode, bool silent = false);
   void Close();

   int64_t GetFileSize();
   void GetBytes(ctDynamicArray<uint8_t>& outArray);
   void GetBytes(ctDynamicArray<char>& outArray);
   void GetText(ctStringUtf8& outString);

   int64_t Tell();
   ctResults Seek(const int64_t offset, const ctFileSeekMode mode);

   size_t ReadRaw(void* pDest, const size_t size, const size_t count);

   size_t WriteRaw(const void* pData, size_t size, const size_t count);
   int64_t Printf(const char* format, ...);
   int64_t VPrintf(const char* format, va_list va);
   void Flush();
   bool isEndOfFile();

   FILE* CFile() const;

   bool isOpen() const;

private:
   ctFileOpenMode _mode;
   int64_t _fSize;
   FILE* _fp;
};