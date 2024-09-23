/*
   Copyright 2024 MacKenzie Strand

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

/* A crude lexer for tokenizing C/C++ style languages */

enum ctLexerTokenType {
   CT_LEXER_TOKEN_IDENT,   /* alphanumeric (ex: keywords, names) */
   CT_LEXER_TOKEN_SYMBOLS, /* a string of symbols */
   CT_LEXER_TOKEN_STRING,  /* string contents, escape codes processed */
   CT_LEXER_TOKEN_CHAR,    /* a single character definition */
   CT_LEXER_TOKEN_COMMENT, /* comment contents (minus delimeters and surrounding space) */
   CT_LEXER_TOKEN_NUMBER,  /* number (converted into double) */
};

class ctLexerToken {
public:
   inline ctLexerToken() = default;

   bool ExpectContents(const char* expected, bool startsWith = false) const;

   inline bool isType(ctLexerTokenType etype) const {
      return GetType() == etype;
   }

   bool isIdentifier(const char* expected = NULL, bool startsWith = false) const {
      if (isType(CT_LEXER_TOKEN_IDENT)) { return ExpectContents(expected, startsWith); }
      return false;
   }

   bool isSymbol(const char* expected = NULL, bool startsWith = false) const {
      if (isType(CT_LEXER_TOKEN_SYMBOLS)) { return ExpectContents(expected, startsWith); }
      return false;
   }

   bool isString(const char* expected = NULL, bool startsWith = false) const {
      if (isType(CT_LEXER_TOKEN_STRING)) { return ExpectContents(expected, startsWith); }
      return false;
   }

   bool isCharacter(const char* expected = NULL) {
      if (isType(CT_LEXER_TOKEN_CHAR)) { return ExpectContents(expected, false); }
      return false;
   }

   bool isComment(const char* expected = NULL, bool startsWith = true) const {
      if (isType(CT_LEXER_TOKEN_COMMENT)) { return ExpectContents(expected, startsWith); }
      return false;
   }

   bool isNumber() const {
      return isType(CT_LEXER_TOKEN_NUMBER);
   }

   inline ctLexerTokenType GetType() const {
      return type;
   }

   inline int32_t GetLineNumber() const {
      return lineNumber;
   }

   inline int32_t GetCharNumber() const {
      return charNumber;
   }

   void GetContents(ctStringUtf8& output, bool append = false) const;
   void GetContents(double& output) const;
   void GetContents(long double& output) const;
   void GetContents(char& output) const;

protected:
   friend class ctLexer;
   inline ctLexerToken(ctLexerTokenType _type,
                       int32_t _lineNumber,
                       int32_t _charNumber,
                       const char* _pBegin,
                       const char* _pEnd) {
      type = _type;
      lineNumber = _lineNumber;
      charNumber = _charNumber;
      pBegin = _pBegin;
      pEnd = _pEnd;
   }

private:
   ctLexerTokenType type;
   int32_t lineNumber;
   int32_t charNumber;
   const char* pBegin;
   const char* pEnd;
};

class ctLexer {
public:
   inline void SetContents(ctFile& file) {
      file.GetText(contents);
   }
   inline void SetContents(ctStringUtf8& string) {
      contents = string;
   }
   inline void SetContents(const char* string, size_t length = 0) {
      ctAssert(string);
      if (!length) {
         contents = string;
      } else {
         contents = ctStringUtf8(string, length);
      }
   }

   ctResults BuildTokens();

   inline size_t GetTokenCount() const {
      return tokens.Count();
   }
   inline ctLexerToken GetToken(size_t index) const {
      return tokens[index];
   }
   inline const ctLexerToken* Begin() const {
      return tokens.Begin();
   }
   inline const ctLexerToken* End() const {
      return tokens.End();
   }

private:
   ctStringUtf8 contents;
   ctDynamicArray<ctLexerToken> tokens;
};

class ctLexerIterator {
public:
   inline ctLexerIterator() = default;
   inline ctLexerIterator(const ctLexerToken* pToken) {
      includeComments = false;
      pHead = pToken;
      pBegin = pToken;
      pEnd = pToken;
   }
   inline ctLexerIterator(const ctLexerToken* pCurrentToken,
                          const ctLexerToken* pStartToken,
                          const ctLexerToken* pEndToken) {
      includeComments = false;
      pHead = pCurrentToken;
      pBegin = pStartToken;
      pEnd = pEndToken;
   }
   inline ctLexerIterator(const ctLexer& lexer) {
      includeComments = false; /* don't nest comment inclusion */
      pHead = lexer.Begin();
      pBegin = lexer.Begin();
      pEnd = lexer.End();
   }

   inline ctLexerIterator& operator++() {
      pHead++;
      if (*this) {
         if (!includeComments && Token().isComment()) { ++(*this); }
      }
      return *this;
   }
   inline ctLexerIterator operator++(int) {
      ctLexerIterator tmp = *this;
      ++*this;
      return tmp;
   }
   inline ctLexerIterator& operator--() {
      pHead--;
      if (*this) {
         if (!includeComments && Token().isComment()) { --(*this); }
      }
      return *this;
   }
   inline ctLexerIterator operator--(int) {
      ctLexerIterator tmp = *this;
      --*this;
      return tmp;
   }
   inline operator bool() const {
      return pHead ? pHead <= pEnd && pHead >= pBegin : false;
   }

   inline const ctLexerToken& Token() {
      ctAssert((*this)); /* iterator must be valid */
      ctAssert(pHead);
      return *pHead;
   }
   inline const ctLexerToken* TokenPtr() {
      return pHead;
   }
   inline const ctLexerToken* BeginPtr() {
      return pBegin;
   }
   inline const ctLexerToken* EndPtr() {
      return pEnd;
   }

   /* including comments allows embedding metadata but can break parsing */
   inline void IncludeComments(bool comments) {
      includeComments = comments;
   }

   /* iterate from current position until before the delimiter */
   ctResults GetDelimeterIterator(ctLexerIterator& iteratorOut,
                                  const char* delimeter = ";",
                                  bool seekEnd = true);
   /* iterate from after the next bracket until before its end point */
   ctResults GetBracketIterator(ctLexerIterator& iteratorOut,
                                const char* open = "{",
                                const char* close = "}",
                                bool seekEnd = true,
                                bool multiLevel = true,
                                bool startSensitive = false);
   /* get the size of this iterator (as a c array), avoids bracket issues */
   size_t GetArraySize(const char* separator = ",", int32_t targetLevel = 0);

   /* seek until the implied subject of a commented line (inline or nextline) */
   ctResults SeekToCommentedLineSubject();

   /* set to the other lexers position if the other lexer is greater
   warning: this copies start and end position as well which may be undesired
   if the iterator is from another lexer, it will cause unexpected behavior */
   bool CopyIfGreater(ctLexerIterator other);

   /* read from current position to end as a C++ variable */
   ctResults ReadAsVariable(
     ctLexerIterator* pNameOut,             /* name of the variable */
     ctLexerIterator* ppTypeOut,            /* type info of the variable */
     ctLexerIterator* pValueItOut = NULL,   /* the contents after = (if any) */
     ctLexerIterator* pArrayInfoOut = NULL, /* contents of [] after type name */
     bool seekEnd = true);

   /* read from current position to end as a C++ type */
   ctResults ReadAsTypeInfo(
     ctLexerIterator* pTypeNameOut, /* type name (may include size/sign and namespaces) */
     ctLexerIterator* pTemplateOut, /* contents of <> template */
     size_t* pPtrLevel = NULL,      /* pointer level */
     size_t* pRefLevel = NULL,      /* reference level */
     bool* pIsConst = NULL,         /* is a const variable */
     bool* pIsStatic = NULL,        /* is a static variable */
     bool seekEnd = true);

   /* read as C variable array info */
   ctResults ReadAsArrayInfo(int32_t& pSizeOut, bool seekEnd = true);

   /* read as the definition of a C/C++ enum */
   ctResults ReadAsEnumDefinition(ctLexerIterator* pNameOut, /* name of the enum */
                                  bool* pIsClass = NULL,
                                  bool seekEnd = true);

   /* read as a C enum entry definition */
   ctResults ReadAsEnumEntry(
     ctLexerIterator* pNameOut,       /* name of the variable */
     ctLexerIterator* pAssignmentOut, /* contents of assignment after = but before , */
     bool seekEnd = true);

   /* read as the definition of a C/C++ struct or class */
   ctResults ReadAsStructDefinition(ctLexerIterator* pNameOut, /* name of the struct */
                                    bool seekEnd = true);

   /* read as the definition of C++ inheritance */
   ctResults ReadAsStructInheritanceInfo(
     ctLexerIterator* pTypeNameOut, /* type name */
       ctLexerIterator* pScopeOut,       /* scope (may be public, private, protected) */
     bool seekEnd = true);

   /* flatten itterator to a string */
   ctStringUtf8 ToString();

   ctResults ExpectToken(const char* identifier, bool seekEnd = true);
   /* attempts to parse token, if it exists seek and return, if not do nothing */
   bool DigestToken(const char* contents = NULL);
   /* passes bracket section, if it exists seek and return, if not do nothing */
   bool DigestBracketSection(const char* open = "{",
                             const char* close = "}",
                             bool multiLevel = true);
   /* digest a C++ type (not including qualifiers, pointers, or references) */
   void DigestCPPTypeName();

   void NextArrayObject();
   ctResults ReadValue(bool& out, bool seekEnd = true);
   ctResults ReadValue(uint8_t& out, bool seekEnd = true);
   ctResults ReadValue(int8_t& out, bool seekEnd = true);
   ctResults ReadValue(uint16_t& out, bool seekEnd = true);
   ctResults ReadValue(int16_t& out, bool seekEnd = true);
   ctResults ReadValue(uint32_t& out, bool seekEnd = true);
   ctResults ReadValue(int32_t& out, bool seekEnd = true);
   ctResults ReadValue(uint64_t& out, bool seekEnd = true);
   ctResults ReadValue(int64_t& out, bool seekEnd = true);
   ctResults ReadValue(float& out, bool seekEnd = true);
   ctResults ReadValue(double& out, bool seekEnd = true);
   ctResults ReadValue(long double& out, bool seekEnd = true);
   ctResults ReadValue(char& out, bool seekEnd = true);
   ctResults ReadValue(ctStringUtf8& out, bool seekEnd = true);
   ctResults ReadValue(ctVec2& out, bool seekEnd = true);
   ctResults ReadValue(ctVec3& out, bool seekEnd = true);
   ctResults ReadValue(ctVec4& out, bool seekEnd = true);
   ctResults ReadValue(ctMat4& out, bool seekEnd = true);
   ctResults ReadValue(ctGUID& out, bool seekEnd = true);
   ctResults ReadValue(ctDynamicArray<uint8_t>& out, bool seekEnd = true);

private:
   ctLexerIterator SaveState() {
      ctLexerIterator result = ctLexerIterator();
      result.pHead = pHead;
      result.pBegin = pBegin;
      result.pEnd = pEnd;
      result.includeComments = includeComments;
      return result;
   }
   void RestoreState(ctLexerIterator& state) {
      includeComments = state.includeComments;
      pHead = state.pHead;
      pBegin = state.pBegin;
      pEnd = state.pEnd;
   }
   bool includeComments;
   const ctLexerToken* pHead;
   const ctLexerToken* pBegin;
   const ctLexerToken* pEnd;
};