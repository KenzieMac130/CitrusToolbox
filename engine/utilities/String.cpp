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

#include "String.hpp"

#include "utf8/utf8.h"
#define CUTE_UTF_IMPLEMENTATION
#include "cute/cute_utf.h"
#include <ctype.h>

/* ------------------------ Global String Pool ------------------------ */

/* ------------------------ String Type ------------------------ */

ctStringUtf8::ctStringUtf8() {
}

ctStringUtf8::~ctStringUtf8() {
}

ctStringUtf8::ctStringUtf8(ctStringUtf8& str) {
   *this += str;
}

ctStringUtf8::ctStringUtf8(const ctStringUtf8& str) {
   *this += str;
}

ctStringUtf8::ctStringUtf8(const char* input, const size_t count) {
   if (!input) { return; }
   const size_t final_count = strnlen(input, count);
   Append(input, count);
}

ctStringUtf8::ctStringUtf8(const char* input) {
   if (!input) { return; }
   const size_t count = strlen(input);
   Append(input, count);
}

ctStringUtf8::ctStringUtf8(const wchar_t* input) {
   if (!input) { return; }
   const wchar_t* next = input;
   while (*next != 0) {
      *this += (int)*next;
      next++;
   }
}

const char* ctStringUtf8::CStr() const {
   return (const char*)_dataVoid();
}

void* ctStringUtf8::Data() const {
   return _dataVoid();
}

size_t ctStringUtf8::CodeLength() const {
   if (isEmpty()) { return 0; }
   return utf8len(_dataVoid());
}

size_t ctStringUtf8::ByteLength() const {
   if (isEmpty()) { return 0; }
   return strnlen(CStr(), Capacity());
}

size_t ctStringUtf8::Capacity() const {
   return _data.Capacity();
}

bool ctStringUtf8::isEmpty() const {
   return _data.isEmpty();
}

ctResults ctStringUtf8::Reserve(const size_t amount) {
   return _data.Reserve(amount);
}

void ctStringUtf8::Clear() {
   _data.Clear();
   _nullTerminate();
}

ctStringUtf8& ctStringUtf8::operator+=(const char c) {
   return Append(&c, 1);
}

ctStringUtf8& ctStringUtf8::operator+=(const int32_t c) {
   char tmp_buf[4] = {0, 0, 0, 0};
   const size_t bytecount =
     (size_t)((const char*)utf8catcodepoint(tmp_buf, c, 4) - tmp_buf);
   return Append(tmp_buf, bytecount);
}

ctStringUtf8& ctStringUtf8::operator+=(const char* str) {
   if (!str) { return *this; }
   return Append(str, strlen(str));
}

ctStringUtf8& ctStringUtf8::operator+=(const ctStringUtf8& str) {
   if (str.isEmpty()) { return *this; }
   return Append(str.CStr(), str.ByteLength());
}

ctStringUtf8& ctStringUtf8::Append(const char* str, const size_t count) {
   if (!str) { return *this; }
   _removeNullTerminator();
   _data.Append(str, count);
   _nullTerminate();
   return *this;
}

ctStringUtf8& ctStringUtf8::Append(const char chr, const size_t count) {
   _removeNullTerminator();
   _data.Append(chr, count);
   _nullTerminate();
   return *this;
}

void ctStringUtf8::Printf(const size_t max, const char* format, ...) {
   if (!format) { return; }
   va_list args;
   va_start(args, format);
   VPrintf(max, format, args);
   va_end(args);
}

void ctStringUtf8::VPrintf(const size_t max, const char* format, va_list args) {
   if (!format) { return; }
   const size_t beginning_length = ByteLength();
   Append('\0', max);
   vsnprintf((char*)_dataVoid() + beginning_length, max, format, args);
}

int ctStringUtf8::Cmp(const ctStringUtf8& str) const {
   return Cmp(str.CStr());
}

int ctStringUtf8::Cmp(const char* str) const {
   if (!str) { return INT32_MIN; }
   return Cmp(str, strlen(str));
}

int ctStringUtf8::Cmp(const char* str, const size_t len) const {
   if (!str) { return INT32_MIN; }
   const size_t maxlen = len > ByteLength() ? ByteLength() : len;
   return utf8ncmp(CStr(), str, maxlen);
}

ctStringUtf8& ctStringUtf8::ToUpper() {
   if (isEmpty()) { return *this; }
   utf8upr(_dataVoid());
   return *this;
}

ctStringUtf8& ctStringUtf8::ToLower() {
   if (isEmpty()) { return *this; }
   utf8lwr(_dataVoid());
   return *this;
}

ctStringUtf8& ctStringUtf8::ProcessEscapeCodes() {
   const size_t len = ByteLength()+1;
   size_t outIdx = 0;
   bool escapeEntered = false;
   for (size_t inIdx = 0; inIdx < len; inIdx++) {
      if (escapeEntered) {
         escapeEntered = false;
         switch (_data[inIdx]) {
            case '\'': _data[outIdx] = '\''; break;
            case '\"': _data[outIdx] = '\"'; break;
            case '?': _data[outIdx] = '\?'; break;
            case '\\': _data[outIdx] = '\\'; break;
            case 'a': continue;
            case 'b': continue;
            case 'f': _data[outIdx] = '\f'; break;
            case 'n': _data[outIdx] = '\n'; break;
            case 'r': _data[outIdx] = '\r'; break;
            case 't': _data[outIdx] = '\t'; break;
            case 'v': _data[outIdx] = '\v'; break;
            default: continue;
         }
      } else if (_data[inIdx] == '\\') {
         escapeEntered = true;
         continue;
      } else {
         _data[outIdx] = _data[inIdx];
      }
      outIdx++;
   }
   _data.Resize(outIdx);
   return *this;
}

bool ctStringUtf8::isNumber() const {
   if (isEmpty()) { return false; }
   for (int i = 0; i < ByteLength(); i++) {
      if (!(_data[i] == '.' || _data[i] == '-' || isdigit(_data[i]))) { return false; }
   }
   return true;
}

bool ctStringUtf8::isInteger() const {
   if (isEmpty()) { return false; }
   for (int i = 0; i < ByteLength(); i++) {
      if (!(_data[i] == '-' || isdigit(_data[i]))) { return false; }
   }
   return true;
}

ctStringUtf8& ctStringUtf8::FilePathUnify() {
   for (int i = 0; i < ByteLength(); i++) {
      if (_data[i] == '\\') { _data[i] = '/'; }
   }
   return *this;
}

#include "system/System.h"

ctStringUtf8& ctStringUtf8::FilePathLocalize() {
   ctSystemFilePathLocalize(_data.Data());
   return *this;
}

ctStringUtf8& ctStringUtf8::FilePathRemoveTrailingSlash() {
   const size_t length = ByteLength();
   if (length < 1) { return *this; }
   if (_data[length - 1] == '/' || _data[length - 1] == '\\') {
      _data[length - 1] = '\0';
      _data.RemoveLast();
   }
   return *this;
}

ctStringUtf8& ctStringUtf8::FilePathRemoveExtension() {
   const size_t length = ByteLength();
   if (length < 1) { return *this; }
   size_t lastDot;
   bool foundDot = false;
   for (lastDot = length - 1; lastDot > 0; lastDot--) {
      if (_data[lastDot] == '.') {
         foundDot = true;
         break;
      }
   }
   if (!foundDot) { return *this; }
   for (size_t i = length - 1; i >= lastDot; i--) {
      if (_data[i] == '/' || _data[i] == '\\') { break; }
      _data[i] = '\0';
      _data.RemoveLast();
   }
   return *this;
}

ctStringUtf8& ctStringUtf8::FilePathPop() {
   const size_t length = ByteLength();
   if (length < 1) { return *this; }
   for (size_t i = length - 1; i > 0; i--) {
      if (_data[i] == '/' || _data[i] == '\\') {
         _data[i] = '\0';
         _data.RemoveLast();
         break;
      }
      _data[i] = '\0';
      _data.RemoveLast();
   }
   return *this;
}

ctStringUtf8& ctStringUtf8::FilePathAppend(const char* path) {
   if (!path) { return *this; }
   if (path[0] == '/' || path[0] == '\\') {
      FilePathRemoveTrailingSlash();
   } else {
      const size_t length = ByteLength();
      if (length) {
         if (_data[length - 1] != '/' && _data[length - 1] != '\\') { *this += "/"; }
      }
   }
   return *this += path;
}

ctStringUtf8& ctStringUtf8::FilePathAppend(const ctStringUtf8& path) {
   return FilePathAppend(path.CStr());
}

ctStringUtf8 ctStringUtf8::FilePathGetName() const {
   const size_t length = ByteLength();
   if (length < 1) { return ""; }
   size_t lastSlash;
   size_t lastDot;
   bool foundSlash = false;
   bool foundDot = false;
   for (lastSlash = length - 1; lastSlash > 0; lastSlash--) {
      if (_data[lastSlash] == '/' || _data[lastSlash] == '\\') {
         foundSlash = true;
         break;
      }
   }
   for (lastDot = length - 1; lastDot > 0; lastDot--) {
      if (_data[lastDot] == '.') {
         foundDot = true;
         break;
      }
   }
   /* No extension */
   if (lastSlash > lastDot || !foundDot && foundSlash) {
      return ctStringUtf8(&CStr()[lastSlash + 1], length - (lastSlash + 1));
   }
   /* Extension */
   else if (foundDot && foundSlash) {
      return ctStringUtf8(&CStr()[lastSlash + 1], lastDot - (lastSlash + 1));
   }
   return "";
}

ctStringUtf8 ctStringUtf8::FilePathGetExtension() const {
   const size_t length = ByteLength();
   if (length < 1) { return ""; }
   size_t lastDot;
   bool foundDot = false;
   for (lastDot = length - 1; lastDot > 0; lastDot--) {
      if (_data[lastDot] == '.') {
         foundDot = true;
         break;
      }
   }
   if (foundDot) { return &CStr()[lastDot]; }
   return "";
}

uint32_t ctStringUtf8::xxHash32(const int seed) const {
   if (isEmpty()) { return 0; }
   return XXH32(_dataVoid(), ByteLength(), seed);
}

uint32_t ctStringUtf8::xxHash32() const {
   return xxHash32(0);
}

uint64_t ctStringUtf8::xxHash64(const int seed) const {
   if (isEmpty()) { return 0; }
   return XXH64(_dataVoid(), ByteLength(), seed);
}

uint64_t ctStringUtf8::xxHash64() const {
   return xxHash64(0);
}

size_t ctStringUtf8::HornerHash() const {
   if (isEmpty()) { return 0; }
   return ctHornerHash(CStr());
}

void ctStringUtf8::MakeUTF16Array(ctDynamicArray<char16_t>& arr) const {
   size_t size = CodeLength();
   arr.Reserve(size);
   arr.Clear();
   void* data = _dataVoid();
   while (1) {
      int32_t chr = 0;
      data = utf8codepoint(data, &chr);
      arr.Append((char16_t)chr);
      if (chr == 0) { break; }
   }
}

void ctStringUtf8::CopyToArray(char* dest, size_t destSize) {
   memset(dest, 0, destSize);
   strncpy(dest, CStr(), destSize - 1);
}

inline void* ctStringUtf8::_dataVoid() const {
   return (void*)_data.Data();
}

void ctStringUtf8::_removeNullTerminator() {
   if (isEmpty()) { return; }
   int64_t idx = _data.Count() - 1;
   if (idx < 0) { idx = 0; }
   while (_data[idx] == '\0') {
      _data.RemoveLast();
      idx--;
      if (idx < 0) { return; }
   }
}

void ctStringUtf8::_nullTerminate() {
   _data.Append('\0');
}

bool operator==(const ctStringUtf8& a, const char* b) {
   if (a.isEmpty() && b == NULL) { return true; }
   if (!a.isEmpty() && b != NULL) { return utf8cmp(a.CStr(), b) == 0; }
   return false;
}

bool operator==(const ctStringUtf8& a, const ctStringUtf8& b) {
   if (a.isEmpty() && b.isEmpty()) { return true; }
   if (!a.isEmpty() && !b.isEmpty()) { return utf8cmp(a.CStr(), b.CStr()) == 0; }
   return false;
}

bool operator==(const char* a, const ctStringUtf8& b) {
   if (a == NULL && b.isEmpty()) { return true; }
   if (a != NULL && !b.isEmpty()) { return utf8cmp(a, b.CStr()) == 0; }
   return false;
}
