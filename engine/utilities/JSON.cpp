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

#define JSMN_PARENT_LINKS
#include "jsmn/jsmn.h"

#include "JSON.hpp"

/*Writer*/

ctJSONWriter::ctJSONWriter() {
   _pStr = 0;
}

void ctJSONWriter::SetStringPtr(ctStringUtf8* pString) {
   _pStr = pString;
   _jsonStack.Clear();
   _pushStack(false);
}

ctResults ctJSONWriter::PushObject() {
   if (!_pStr) { return CT_FAILURE_DATA_DOES_NOT_EXIST; }
   _finishLastEntry();
   *_pStr += "\n{";
   _unmarkFirst();
   _setDefinition(false);
   _pushStack(false);
   return CT_SUCCESS;
}

ctResults ctJSONWriter::PopObject() {
   if (!_pStr) { return CT_FAILURE_DATA_DOES_NOT_EXIST; }
   *_pStr += "\n}";
   _popStack();
   return CT_SUCCESS;
}

ctResults ctJSONWriter::PushArray() {
   if (!_pStr) { return CT_FAILURE_DATA_DOES_NOT_EXIST; }
   _finishLastEntry();
   *_pStr += "[";
   _unmarkFirst();
   _setDefinition(false);
   _pushStack(true);
   return CT_SUCCESS;
}

ctResults ctJSONWriter::PopArray() {
   if (!_pStr) { return CT_FAILURE_DATA_DOES_NOT_EXIST; }
   *_pStr += "]";
   _popStack();
   return CT_SUCCESS;
}

ctResults ctJSONWriter::DeclareVariable(const char* name) {
   if (!_pStr) { return CT_FAILURE_DATA_DOES_NOT_EXIST; }
   _finishLastEntry();
   _pStr->Printf(strlen(name) + 6, "\"%s\": ", name);
   _setDefinition(true);
   return CT_SUCCESS;
}

ctResults ctJSONWriter::WriteString(const char* value) {
   if (!_pStr) { return CT_FAILURE_DATA_DOES_NOT_EXIST; }
   _finishLastEntry();
   _pStr->Printf(strlen(value) + 4, "\"%s\"", value);
   _unmarkFirst();
   _setDefinition(false);
   return CT_SUCCESS;
}

ctResults ctJSONWriter::WriteNumber(double value) {
   if (!_pStr) { return CT_FAILURE_DATA_DOES_NOT_EXIST; }
   _finishLastEntry();
   _pStr->Printf(32, "%f", value);
   _unmarkFirst();
   _setDefinition(false);
   return CT_SUCCESS;
}

ctResults ctJSONWriter::WriteNumber(int64_t value) {
   if (!_pStr) { return CT_FAILURE_DATA_DOES_NOT_EXIST; }
   _finishLastEntry();
   _pStr->Printf(32, "%" PRId64, value);
   _unmarkFirst();
   _setDefinition(false);
   return CT_SUCCESS;
}

ctResults ctJSONWriter::WriteNumber(int32_t value) {
   if (!_pStr) { return CT_FAILURE_DATA_DOES_NOT_EXIST; }
   _finishLastEntry();
   _pStr->Printf(32, "%" PRId32, value);
   _unmarkFirst();
   _setDefinition(false);
   return CT_SUCCESS;
}

ctResults ctJSONWriter::WriteBool(bool value) {
   if (!_pStr) { return CT_FAILURE_DATA_DOES_NOT_EXIST; }
   _finishLastEntry();
   if (value) {
      *_pStr += "true";
   } else {
      *_pStr += "false";
   }
   _unmarkFirst();
   _setDefinition(false);
   return CT_SUCCESS;
}

ctResults ctJSONWriter::WriteNull() {
   if (!_pStr) { return CT_FAILURE_DATA_DOES_NOT_EXIST; }
   _finishLastEntry();
   *_pStr += "null";
   _unmarkFirst();
   _setDefinition(false);
   return CT_SUCCESS;
}

void ctJSONWriter::_finishLastEntry() {
   const ctJSONWriter::_json_stack state = _jsonStack.Last();
   if (state.isDefinition) { return; }
   if (!state.isFirst) { *_pStr += ","; }
   if (!state.isArray) { *_pStr += "\n"; }
}

void ctJSONWriter::_unmarkFirst() {
   _jsonStack.Last().isFirst = false;
}

void ctJSONWriter::_setDefinition(bool val) {
   _jsonStack.Last().isDefinition = val;
}

void ctJSONWriter::_pushStack(bool isArray) {
   ctJSONWriter::_json_stack state;
   state.isFirst = true;
   state.isDefinition = false;
   state.isArray = isArray;
   _jsonStack.Append(state);
}

void ctJSONWriter::_popStack() {
   if (_jsonStack.Count() > 1) { _jsonStack.RemoveLast(); }
}

/*Parser*/

ctResults ctJSONReader::BuildJsonForPtr(const char* pData, size_t length) {
   _pData = pData;
   jsmn_parser parser;
   jsmn_init(&parser);
   int tokenCount = jsmn_parse(&parser, pData, length, NULL, 0);
   if (tokenCount == 0) { return CT_FAILURE_CORRUPTED_CONTENTS; }
   _tokens.Resize(tokenCount);
   jsmn_init(&parser);
   jsmn_parse(&parser, pData, length, _tokens.Data(), tokenCount);
   return CT_SUCCESS;
}

void ctJSONReader::GetRootEntry(Entry& entry) {
   if (_tokens.Count() > 0) {
      entry =
        Entry(0, (int)_tokens.Count(), _tokens[0], _tokens.Data(), _pData);
   }
}

ctJSONReader::Entry::Entry() {
   jsmn_fill_token(&_token, JSMN_UNDEFINED, 0, 0);
   _pTokens = NULL;
   _pData = NULL;
   _tokenPos = 0;
   _tokenCount = 0;
}

ctJSONReader::Entry::Entry(int pos,
                           int count,
                           jsmntok_t token,
                           jsmntok_t* pTokenArr,
                           const char* pData) {
   _tokenPos = pos;
   _tokenCount = count;
   _token = token;
   _pTokens = pTokenArr;
   _pData = pData;
}

size_t ctJSONReader::Entry::GetRaw(char* pDest, int size) {
   if (!_pData) { return 0; }
   if (!pDest) { return (size_t)_token.end - _token.start; }
   size =
     size > (_token.end - _token.start) ? (_token.end - _token.start) : size;
   if (pDest) { strncpy(pDest, &_pData[_token.start], size); }
   return size;
}

ctResults ctJSONReader::Entry::_getEntry(int index, Entry& entry) {
   if (index < 0 || index >= _tokenCount) { return CT_FAILURE_OUT_OF_BOUNDS; }
   entry = Entry(index, _tokenCount, _pTokens[index], _pTokens, _pData);
   return CT_SUCCESS;
}

ctResults ctJSONReader::Entry::GetObjectEntry(const char* name, Entry& entry) {
   if (!isObject()) { return CT_FAILURE_PARSE_ERROR; }
   for (int i = _tokenPos; i < _tokenCount; i++) {
      const jsmntok_t tok = _pTokens[i];
      const char* str = &_pData[tok.start];
      if (tok.parent != _tokenPos) { continue; }
      if (strncmp(name, str, (size_t)tok.end - tok.start) == 0) {
         return _getEntry(i + 1, entry);
      }
   }
   return CT_FAILURE_DATA_DOES_NOT_EXIST;
}

int ctJSONReader::Entry::ObjectEntryCount() {
   return _token.size;
}

ctResults ctJSONReader::Entry::GetObjectEntryIdx(int index,
                                                 Entry& entry,
                                                 ctStringUtf8* pLabel) {
   if (!isObject()) { return CT_FAILURE_PARSE_ERROR; }
   int occurrance = 0;
   for (int i = _tokenPos; i < _tokenCount; i++) {
      const jsmntok_t tok = _pTokens[i];
      const char* str = &_pData[tok.start];
      if (tok.parent != _tokenPos) { continue; }
      if (occurrance == index) {
         if (pLabel) {
            *pLabel =
              ctStringUtf8(&_pData[tok.start], (size_t)tok.end - tok.start);
         }
         return _getEntry(i + 1, entry);
      }
      occurrance++;
   }
   return CT_FAILURE_DATA_DOES_NOT_EXIST;
}

ctResults ctJSONReader::Entry::GetArrayEntry(int index, Entry& entry) {
   if (!isArray()) { return CT_FAILURE_PARSE_ERROR; }
   if (index > ArrayLength()) { return CT_FAILURE_OUT_OF_BOUNDS; }
   return _getEntry(_tokenPos + 1 + index, entry);
}

int ctJSONReader::Entry::ArrayLength() {
   return _token.size;
}

bool ctJSONReader::Entry::isArray() {
   return _token.type == JSMN_ARRAY;
}

bool ctJSONReader::Entry::isObject() {
   return _token.type == JSMN_OBJECT;
}

bool ctJSONReader::Entry::isString() {
   return _token.type == JSMN_STRING;
}

bool ctJSONReader::Entry::isPrimitive() {
   return _token.type == JSMN_PRIMITIVE;
}

bool ctJSONReader::Entry::isNull() {
   return isPrimitive() && _pData[_token.start] == 'n';
}

bool ctJSONReader::Entry::isBool() {
   return isPrimitive() &&
          (_pData[_token.start] == 't' || _pData[_token.start] == 'f');
}

bool ctJSONReader::Entry::isNumber() {
   return isPrimitive() &&
          (_pData[_token.start] == '-' || _pData[_token.start] == '0' ||
           _pData[_token.start] == '1' || _pData[_token.start] == '2' ||
           _pData[_token.start] == '3' || _pData[_token.start] == '4' ||
           _pData[_token.start] == '5' || _pData[_token.start] == '6' ||
           _pData[_token.start] == '7' || _pData[_token.start] == '8' ||
           _pData[_token.start] == '9');
}

void ctJSONReader::Entry::GetString(ctStringUtf8& out) {
   out = ctStringUtf8(&_pData[_token.start], (size_t)_token.end - _token.start);
}

void ctJSONReader::Entry::GetString(char* pDest, size_t max) {
   const size_t srcSize = (size_t)_token.end - _token.start;
   const size_t dstSize = max;
   const size_t size = srcSize > dstSize ? dstSize : srcSize;
   strncpy(pDest, &_pData[_token.start], size);
}

ctResults ctJSONReader::Entry::GetBool(bool& out) {
   if (!isBool()) { return CT_FAILURE_PARSE_ERROR; }
   if (_pData[_token.start] == 't') {
      out = true;
   } else if (_pData[_token.start] == 'f') {
      out = false;
   }
   return CT_SUCCESS;
}

#define GETNSTR()                                                              \
   if (!isNumber()) { return CT_FAILURE_PARSE_ERROR; }                         \
   char str[32];                                                               \
   memset(str, 0, 32);                                                         \
   GetString(str, 31);

ctResults ctJSONReader::Entry::GetNumber(float& out) {
   GETNSTR();
   out = (float)atof(str);
   return CT_SUCCESS;
}

ctResults ctJSONReader::Entry::GetNumber(double& out) {
   GETNSTR();
   out = (double)atof(str);
   return CT_SUCCESS;
}

ctResults ctJSONReader::Entry::GetNumber(int8_t& out) {
   GETNSTR();
   out = (int8_t)atoi(str);
   return CT_SUCCESS;
}

ctResults ctJSONReader::Entry::GetNumber(int16_t& out) {
   GETNSTR();
   out = (int16_t)atoi(str);
   return CT_SUCCESS;
}

ctResults ctJSONReader::Entry::GetNumber(int32_t& out) {
   GETNSTR();
   out = (int32_t)atoi(str);
   return CT_SUCCESS;
}

ctResults ctJSONReader::Entry::GetNumber(int64_t& out) {
   GETNSTR();
   out = (int64_t)atoll(str);
   return CT_SUCCESS;
}

ctResults ctJSONReader::Entry::GetNumber(uint8_t& out) {
   GETNSTR();
   out = (uint8_t)atoi(str);
   return CT_SUCCESS;
}

ctResults ctJSONReader::Entry::GetNumber(uint16_t& out) {
   GETNSTR();
   out = (uint16_t)atoi(str);
   return CT_SUCCESS;
}

ctResults ctJSONReader::Entry::GetNumber(uint32_t& out) {
   GETNSTR();
   out = (uint32_t)atoi(str);
   return CT_SUCCESS;
}

ctResults ctJSONReader::Entry::GetNumber(uint64_t& out) {
   GETNSTR();
   out = (uint64_t)atoll(str);
   return CT_SUCCESS;
}
