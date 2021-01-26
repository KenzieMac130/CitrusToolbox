#include "jsmn/jsmn.h"

#include "JSON.hpp"

/*Writer*/

ctJSONWriter::ctJSONWriter() {
   _pStr = 0;
   _objLevel = 0;
   _arrLevel = 0;
}

void ctJSONWriter::SetStringPtr(ctStringUtf8* pString) {
   _pStr = pString;
   _objLevel = 0;
   _arrLevel = 0;
   _jsonStack.Clear();
   _pushStack(false);
}

ctResults ctJSONWriter::PushObject() {
   if (!_pStr) { return CT_FAILURE_DATA_DOES_NOT_EXIST; }
   _finishLastEntry();
   *_pStr += "\n{";
   _objLevel++;
   _unmarkFirst();
   _setDefinition(false);
   _pushStack(false);
   return CT_SUCCESS;
}

ctResults ctJSONWriter::PopObject() {
   if (!_pStr) { return CT_FAILURE_DATA_DOES_NOT_EXIST; }
   *_pStr += "\n}";
   _objLevel--;
   _popStack();
   return CT_SUCCESS;
}

ctResults ctJSONWriter::PushArray() {
   if (!_pStr) { return CT_FAILURE_DATA_DOES_NOT_EXIST; }
   _finishLastEntry();
   *_pStr += "[";
   _arrLevel++;
   _unmarkFirst();
   _setDefinition(false);
   _pushStack(true);
   return CT_SUCCESS;
}

ctResults ctJSONWriter::PopArray() {
   if (!_pStr) { return CT_FAILURE_DATA_DOES_NOT_EXIST; }
   *_pStr += "]";
   _arrLevel--;
   _popStack();
   return CT_SUCCESS;
}

ctResults ctJSONWriter::DeclareVariable(const ctStringUtf8& name) {
   if (!_pStr) { return CT_FAILURE_DATA_DOES_NOT_EXIST; }
   _finishLastEntry();
   _pStr->Printf(name.ByteLength() + 6, "\"%s\": ", name.CStr());
   _setDefinition(true);
   return CT_SUCCESS;
}

ctResults ctJSONWriter::WriteString(const ctStringUtf8& value) {
   if (!_pStr) { return CT_FAILURE_DATA_DOES_NOT_EXIST; }
   _finishLastEntry();
   _pStr->Printf(value.ByteLength() + 4, "\"%s\"", value.CStr());
   _unmarkFirst();
   _setDefinition(false);
   return CT_SUCCESS;
}

ctResults ctJSONWriter::WriteNumber(double value) {
   if (!_pStr) { return CT_FAILURE_DATA_DOES_NOT_EXIST; }
   _finishLastEntry();
   _pStr->Printf(32 + 2, "\"%f\"", value);
   _unmarkFirst();
   _setDefinition(false);
   return CT_SUCCESS;
}

ctResults ctJSONWriter::WriteNumber(int64_t value) {
   if (!_pStr) { return CT_FAILURE_DATA_DOES_NOT_EXIST; }
   _finishLastEntry();
   _pStr->Printf(32 + 2, "\"%" PRId64 "\"", value);
   _unmarkFirst();
   _setDefinition(false);
   return CT_SUCCESS;
}

ctResults ctJSONWriter::WriteNumber(int32_t value) {
   if (!_pStr) { return CT_FAILURE_DATA_DOES_NOT_EXIST; }
   _finishLastEntry();
   _pStr->Printf(32 + 2, "\"%" PRId32 "\"", value);
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

ctResults ctJSONWriter::Validate() {
   if (_objLevel == 0 && _arrLevel == 0) { return CT_SUCCESS; }
   return CT_FAILURE_PARSE_ERROR;
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
      entry = Entry(_tokens[0], _tokens.Data(), _tokens.Count(), _pData);
   }
}

ctJSONReader::Entry::Entry() {
   jsmn_fill_token(&_token, JSMN_UNDEFINED, 0, 0);
   _pTokenArr = NULL;
   _tokenArrCount = 0;
   _pData = NULL;
}

ctJSONReader::Entry::Entry(jsmntok_t token,
                           jsmntok_t* pTokenArr,
                           size_t tokenArrCount,
                           const char* pData) {
   _token = token;
   _pTokenArr = pTokenArr;
   _tokenArrCount = tokenArrCount;
   _pData = pData;
}

size_t ctJSONReader::Entry::GetRaw(char* pDest, int size) {
   if (!_pData) { return 0; }
   if (!pDest) { return _token.end - _token.start; }
   size =
     size > (_token.end - _token.start) ? (_token.end - _token.start) : size;
   if (pDest) { strncpy(pDest, &_pData[_token.start], size); }
   return size;
}

bool ctJSONReader::Entry::isObject() {
   return _token.type == JSMN_OBJECT;
}

bool ctJSONReader::Entry::isArray() {
   return _token.type == JSMN_ARRAY;
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

bool ctJSONReader::Entry::isNumber()
{
    return isPrimitive();
}
