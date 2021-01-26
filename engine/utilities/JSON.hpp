#pragma once

#include "Common.h"
#include "String.hpp"

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

   ctResults Validate();

private:
   void _finishLastEntry();
   void _unmarkFirst();
   void _setDefinition(bool val);
   void _pushStack(bool isArray);
   void _popStack();
   int _objLevel;
   int _arrLevel;
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
      Entry(jsmntok_t token,
            jsmntok_t* pTokenArr,
            size_t tokenArrCount,
            const char* pData);

      size_t GetRaw(char* pDest, int size);

      bool isObject();
      void ObjectEntry(const char* name, Entry& entry);

      bool isArray();
      void ArrayEntry(int index, Entry& entry);

      bool isString();
      bool isPrimitive();
      bool isNull();
      bool isBool();
      bool isNumber();
      void GetString(ctStringUtf8& out);
      void GetString();
      void GetBool();
      void GetNumber();
      /*Todo: Vector Math*/

   protected:
      jsmntok_t _token;
      jsmntok_t* _pTokenArr;
      size_t _tokenArrCount;
      const char* _pData;
   };

   void GetRootEntry(Entry& entry);

private:
   const char* _pData;
   ctDynamicArray<jsmntok_t> _tokens;
};