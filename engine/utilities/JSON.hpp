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
   ctResults DeclareVariable(const ctStringUtf8& name);
   ctResults WriteString(const ctStringUtf8& value);
   ctResults WriteNumber(double value);
   ctResults WriteNumber(int64_t value);
   ctResults WriteNumber(int32_t value);
   ctResults WriteBool(bool value);
   ctResults WriteNull();
   /*Todo: Vector Math*/

private:
   void _finishLastEntry();
   void _unmarkFirst();
   void _setDefinition(bool val);
   void _pushStack(bool isArray);
   void _popStack();
   ctStringUtf8* _pStr;

   struct _json_stack {
      bool isDefinition;
      bool isFirst;
      bool isArray;
   };
   ctStaticArray<_json_stack, 32> _jsonStack;
};

class ctJSONReader {
public:
   ctResults BuildJsonForPtr(const char* pData, size_t length);

   class Entry {
   public:
      Entry();
      Entry(int id, int count, jsmntok_t token, jsmntok_t* pTokenArr, const char* pData);

      size_t GetRaw(char* pDest, int size);

      bool isObject();
      ctResults GetObjectEntry(const char* name, Entry& entry);

      bool isArray();
      ctResults GetArrayEntry(int index, Entry& entry);
      int ArrayLength();
      /*Todo: Vector Math*/

      bool isString();
      bool isPrimitive();
      bool isNull();
      bool isBool();
      bool isNumber();
      void GetString(ctStringUtf8& out);
      void GetString(char* pDest, size_t max);
      ctResults GetBool(bool& out);
      ctResults GetNumber(float& out);
      ctResults GetNumber(double& out);
      ctResults GetNumber(int8_t& out);
      ctResults GetNumber(int16_t& out);
      ctResults GetNumber(int32_t& out);
      ctResults GetNumber(int64_t& out);
      ctResults GetNumber(uint8_t& out);
      ctResults GetNumber(uint16_t& out);
      ctResults GetNumber(uint32_t& out);
      ctResults GetNumber(uint64_t& out);

   protected:
       ctResults _getEntry(int index, Entry& entry);
      jsmntok_t _token;
      int _tokenPos;
      int _tokenCount;
      jsmntok_t* _pTokens;
      const char* _pData;
   };

   void GetRootEntry(Entry& entry);

private:
   const char* _pData;
   ctDynamicArray<jsmntok_t> _tokens;
};