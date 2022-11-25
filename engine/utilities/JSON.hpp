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

#pragma once

#include "Common.h"
#include "String.hpp"

#define JSMN_PARENT_LINKS
#define JSMN_HEADER
#include "jsmn/jsmn.h"

class ctStringUtf8;

class ctJSONWriter {
public:
   ctJSONWriter();
   void SetStringPtr(ctStringUtf8* pString);
   ctResults PushObject();
   ctResults PopObject();
   ctResults PushArray();
   ctResults PopArray();
   ctResults DeclareVariable(const char* name);
   ctResults WriteString(const char* value);
   ctResults WriteNumber(double value);
   ctResults WriteNumber(int32_t value);
   ctResults WriteNumber(int64_t value);
   ctResults WriteBool(bool value);
   ctResults WriteNull();

private:
   void _finishLastEntry();
   void _unmarkFirst();
   void _setDefinition(bool val);
   void _pushStack(bool isArray);
   void _popStack();
   void _makeIndents();
   ctStringUtf8* _pStr;

   struct _json_stack {
      bool isDefinition;
      bool isFirst;
      bool isArray;
   };
   uint32_t indentLevel;
   bool started;
   size_t _jsonStackCt;
   _json_stack _jsonStack[32];
};

class ctJSONReadEntry {
public:
   ctJSONReadEntry();
   ctJSONReadEntry(
     int id, int count, jsmntok_t token, jsmntok_t* pTokenArr, const char* pData);

   bool isValid();

   size_t GetRaw(char* pDest, int size) const;

   bool isObject() const;
   int GetObjectEntryCount() const;
   ctResults GetObjectEntry(const char* name, ctJSONReadEntry& entry) const;
   ctResults
   GetObjectEntry(int idx, ctJSONReadEntry& entry, ctStringUtf8* pLabel = NULL) const;

   bool isArray() const;
   int GetArrayLength() const;
   ctResults GetArrayEntry(int index, ctJSONReadEntry& entry) const;

   bool isString() const;
   bool isPrimitive() const;
   bool isNull() const;
   bool isBool() const;
   bool isNumber() const;
   void GetString(ctStringUtf8& out) const;
   void GetString(char* pDest, size_t max) const;
   ctResults GetBool(bool& out) const;
   ctResults GetNumber(float& out) const;
   ctResults GetNumber(double& out) const;
   ctResults GetNumber(int8_t& out) const;
   ctResults GetNumber(int16_t& out) const;
   ctResults GetNumber(int32_t& out) const;
   ctResults GetNumber(int64_t& out) const;
   ctResults GetNumber(uint8_t& out) const;
   ctResults GetNumber(uint16_t& out) const;
   ctResults GetNumber(uint32_t& out) const;
   ctResults GetNumber(uint64_t& out) const;

protected:
   inline int _getActualLength() const;
   inline ctResults _getEntry(int index, ctJSONReadEntry& entry) const;
   jsmntok_t _token;
   int _tokenPos;
   int _tokenCount;
   jsmntok_t* _pTokens;
   const char* _pData;
};

class ctJSONReader {
public:
   ctResults BuildJsonForPtr(const char* pData, size_t length);

   ctResults GetRootEntry(ctJSONReadEntry& entry);

private:
   const char* _pData;
   ctDynamicArray<jsmntok_t> _tokens;
};