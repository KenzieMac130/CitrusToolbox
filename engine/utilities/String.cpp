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

#include "String.hpp"

#include "utf8/utf8.h"
#define CUTE_UTF_IMPLEMENTATION
#include "cute/cute_utf.h"

ctStringUtf8::ctStringUtf8() {
}

ctStringUtf8::ctStringUtf8(ctStringUtf8& str) {
   *this += str;
}

ctStringUtf8::ctStringUtf8(const ctStringUtf8& str) {
   *this += str;
}

ctStringUtf8::ctStringUtf8(const char* input, const size_t count) {
   const size_t final_count = strnlen(input, count);
   Append(input, count);
}

ctStringUtf8::ctStringUtf8(const char* input) {
   const size_t count = strlen(input);
   Append(input, count);
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
   return Append(str, strlen(str));
}

ctStringUtf8& ctStringUtf8::operator+=(const ctStringUtf8& str) {
   return Append(str.CStr(), str.ByteLength());
}

ctStringUtf8& ctStringUtf8::Append(const char* str, size_t count) {
   _removeNullTerminator();
   _data.Append(str, count);
   _nullTerminate();
   return *this;
}

ctStringUtf8& ctStringUtf8::Append(const char chr, size_t count) {
   _removeNullTerminator();
   _data.Append(chr, count);
   _nullTerminate();
   return *this;
}

void ctStringUtf8::Printf(size_t max, const char* format, ...) {
   va_list args;
   va_start(args, format);
   VPrintf(max, format, args);
   va_end(args);
}

void ctStringUtf8::VPrintf(size_t max, const char* format, va_list args) {
   const size_t beginning_length = ByteLength();
   Append('\0', max);
   vsnprintf((char*)_dataVoid() + beginning_length, max, format, args);
}

int ctStringUtf8::Cmp(const ctStringUtf8& str) const {
   return utf8cmp(CStr(), str.CStr());
}

int ctStringUtf8::Cmp(const char* str) const {
   return utf8cmp(CStr(), str);
}

int ctStringUtf8::Cmp(const char* str, const size_t len) const {
   return utf8ncmp(CStr(), str, len);
}

ctStringUtf8& ctStringUtf8::ToUpper() {
   utf8upr(_dataVoid());
   return *this;
}

ctStringUtf8& ctStringUtf8::ToLower() {
   utf8lwr(_dataVoid());
   return *this;
}

ctStringUtf8& ctStringUtf8::FilePathUnify() {
   for (int i = 0; i < ByteLength(); i++) {
      if (_data[i] == '\\') { _data[i] = '/'; }
   }
   return *this;
}

ctStringUtf8& ctStringUtf8::FilePathLocalize() {
   for (int i = 0; i < ByteLength(); i++) {
#if defined(WIN32) || defined(_WIN32) ||                                       \
  defined(__WIN32) && !defined(__CYGWIN__)
      if (_data[i] == '/') { _data[i] = '\\'; }
#else
      if (_data[i] == '\\') { _data[i] = '/'; }
#endif
   }
   return *this;
}

uint32_t ctStringUtf8::xxHash32(const int seed) const {
   return XXH32(_dataVoid(), ByteLength(), seed);
}

uint32_t ctStringUtf8::xxHash32() const {
   return xxHash32(0);
}

uint64_t ctStringUtf8::xxHash64(const int seed) const {
   return XXH64(_dataVoid(), ByteLength(), seed);
}

uint64_t ctStringUtf8::xxHash64() const {
   return xxHash64(0);
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

bool operator==(ctStringUtf8& a, const char* b) {
   return utf8cmp(a.CStr(), b) == 0;
}

bool operator==(ctStringUtf8& a, ctStringUtf8& b) {
   return utf8cmp(a.CStr(), b.CStr()) == 0;
}

bool operator==(const char* a, ctStringUtf8& b) {
   return utf8cmp(a, b.CStr()) == 0;
}
