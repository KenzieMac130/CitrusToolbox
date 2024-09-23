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

#include "Lexer.hpp"
#include <ctype.h>

bool isNewline(const char* head) {
   return head[0] == '\r' || head[0] == '\n';
}

bool isWhiteSpace(const char* head, bool includeNewline = true) {
   switch (head[0]) {
      case ' ': return true;
      case '\t': return true;
      case '\r': return includeNewline;
      case '\n': return includeNewline;
      default: return false;
   }
}

/* sometimes lines contain both \n and \r and we want to skip past it*/
void seekHead(const char** pHead,
              const char** pLineStart,
              uint32_t* pLineNumber,
              int increment) {
   int direction = increment >= 0 ? 1 : -1;
   while (increment != 0) {
      *pHead += direction;
      increment -= direction;

      /* handle line changes */
      bool lineChanged = false;
      if (*pHead[0] == '\r') {
         lineChanged = true;
         *pHead += direction;
      }
      if (*pHead[0] == '\n') {
         lineChanged = true;
         *pHead += direction;
      }
      if (lineChanged) {
         *pLineStart = *pHead;
         *pLineNumber += direction;
      }
   }
}

#define BUILD_TOKEN_GUARD()                                                              \
   if (!head) { return CT_SUCCESS; }                                                     \
   if (!head[0]) { return CT_SUCCESS; }

ctResults ctLexer::BuildTokens() {
   tokens.Clear();
   if (contents.isEmpty()) { return CT_FAILURE_PARSE_ERROR; }
   uint32_t lineNumber = 1;
   const char* lineStart = contents.CStr();
   for (const char* head = contents.CStr(); *head;
        seekHead(&head, &lineStart, &lineNumber, 1)) {
      /* handle single-line comments */
      BUILD_TOKEN_GUARD()
      if (head[0] == '/' && head[1] == '/') {
         int32_t commentLineNumber = lineNumber;
         int32_t commentPosition = (int32_t)(head - lineStart);
         seekHead(&head, &lineStart, &lineNumber, 2);
         const char* pCommentBegin = head;
         const char* pCommentEnd = NULL;
         /* find beginning of comment minus whitespace */
         while (isWhiteSpace(head)) {
            seekHead(&head, &lineStart, &lineNumber, 1);
            pCommentBegin = head;
         }
         /* digest comment contents */
         while (*head) {
            /* find the end of the comment */
            if (lineNumber != commentLineNumber) {
               /* trim whitespace from end of comment */
               pCommentEnd = &head[0];
               while (isWhiteSpace(pCommentEnd)) {
                  pCommentEnd--;
               }
               pCommentEnd++;
               seekHead(&head, &lineStart, &lineNumber, 1);
               break;
            }
            seekHead(&head, &lineStart, &lineNumber, 1);
         }
         if (!pCommentEnd) { pCommentEnd = head; } /* comment ends at eof */
         tokens.Append(ctLexerToken(CT_LEXER_TOKEN_COMMENT,
                                    commentLineNumber,
                                    commentPosition,
                                    pCommentBegin,
                                    pCommentEnd));
      }

      /* handle multi-line comments */
      BUILD_TOKEN_GUARD()
      if (head[0] == '/' && head[1] == '*') {
         int32_t commentLineNumber = lineNumber;
         int32_t commentPosition = (int32_t)(head - lineStart);
         seekHead(&head, &lineStart, &lineNumber, 2);
         const char* pCommentBegin = head;
         const char* pCommentEnd = NULL;
         /* find beginning of comment minus whitespace */
         while (isWhiteSpace(head)) {
            seekHead(&head, &lineStart, &lineNumber, 1);
            pCommentBegin = head;
         }
         /* digest comment contents */
         while (*head) {
            /* find the end of the comment */
            if (head[0] == '*' && head[1] == '/') {
               /* trim whitespace from end of comment */
               pCommentEnd = &head[-1];
               while (isWhiteSpace(pCommentEnd)) {
                  pCommentEnd--;
               }
               pCommentEnd++;
               seekHead(&head, &lineStart, &lineNumber, 2);
               break;
            }
            seekHead(&head, &lineStart, &lineNumber, 1);
         }
         if (!pCommentEnd) { return CT_FAILURE_PARSE_ERROR; }
         tokens.Append(ctLexerToken(CT_LEXER_TOKEN_COMMENT,
                                    commentLineNumber,
                                    commentPosition,
                                    pCommentBegin,
                                    pCommentEnd));
      }

      /* handle strings */
      BUILD_TOKEN_GUARD()
      if (head[0] == '\"') {
         int32_t stringLineNumber = lineNumber;
         int32_t stringPosition = (int32_t)(head - lineStart);
         const char* pStringBegin = head + 1;
         const char* pStringEnd = NULL;
         while (*head) {
            if (head[0] == '\\') {
               seekHead(&head, &lineStart, &lineNumber, 2);
            } else {
               seekHead(&head, &lineStart, &lineNumber, 1);
            }
            if (head[0] == '\"') {
               pStringEnd = head;
               seekHead(&head, &lineStart, &lineNumber, 1);
               break;
            }
         }
         if (!pStringEnd) { return CT_FAILURE_PARSE_ERROR; }
         tokens.Append(ctLexerToken(CT_LEXER_TOKEN_STRING,
                                    stringLineNumber,
                                    stringPosition,
                                    pStringBegin,
                                    pStringEnd));
      }

      /* handle chars */
      BUILD_TOKEN_GUARD()
      if (head[0] == '\'') {
         int32_t stringLineNumber = lineNumber;
         int32_t stringPosition = (int32_t)(head - lineStart);
         const char* pStringBegin = head + 1;
         const char* pStringEnd = NULL;
         while (*head) {
            if (head[0] == '\\') {
               seekHead(&head, &lineStart, &lineNumber, 2);
            } else {
               seekHead(&head, &lineStart, &lineNumber, 1);
            }
            if (head[0] == '\'') {
               pStringEnd = head;
               seekHead(&head, &lineStart, &lineNumber, 1);
               break;
            }
         }
         if (!pStringEnd) { return CT_FAILURE_PARSE_ERROR; }
         tokens.Append(ctLexerToken(CT_LEXER_TOKEN_CHAR,
                                    stringLineNumber,
                                    stringPosition,
                                    pStringBegin,
                                    pStringEnd));
      }

      /* handle identifiers (alnum) */
      BUILD_TOKEN_GUARD()
      if (ctIsAlphaUnicode(head[0]) || head[0] == '_' || head[0] == '#') {
         int32_t identLineNumber = lineNumber;
         int32_t identPosition = (int32_t)(head - lineStart);
         const char* pIdentBegin = head;
         const char* pIdentEnd = NULL;
         while (*head) {
            if (!ctIsAlnumUnicode(head[0]) && head[0] != '_' && head[0] != '#') {
               pIdentEnd = head;
               break;
            }
            seekHead(&head, &lineStart, &lineNumber, 1);
         }
         if (!pIdentEnd) { pIdentEnd = head; } /* end of string may be \0 */
         tokens.Append(ctLexerToken(
           CT_LEXER_TOKEN_IDENT, identLineNumber, identPosition, pIdentBegin, pIdentEnd));
      }

      /* handle numbers */
      BUILD_TOKEN_GUARD()
      if ((head[0] == '.' && ctIsDigit(head[1])) ||
          (head[0] == '-' && (ctIsDigit(head[1]) || head[1] == '.')) ||
          ctIsDigit(head[0])) {
         int32_t numberLineNumber = lineNumber;
         int32_t numberPosition = (int32_t)(head - lineStart);
         const char* pNumberBegin = head;
         const char* pNumberEnd = NULL;
         seekHead(&head, &lineStart, &lineNumber, 1);
         while (*head) {
            if (!ctIsAlnumUnicode(head[0]) && head[0] != '.') {
               pNumberEnd = head;
               break;
            }
            seekHead(&head, &lineStart, &lineNumber, 1);
         }
         if (!pNumberEnd) { pNumberEnd = head; } /* numbers shouldn't cause failures */
         tokens.Append(ctLexerToken(CT_LEXER_TOKEN_NUMBER,
                                    numberLineNumber,
                                    numberPosition,
                                    pNumberBegin,
                                    pNumberEnd));
      }

      /* handle symbols (https://en.cppreference.com/w/cpp/language/expressions and $) */
      // clang-format off
      const char* knownSymbols[] = {"<<=", /* 3 long */
                                    ">>=",
                                    "<=>",
                                    "->*",
                                     "+=", /* 2 long */
                                    "-=",
                                    "*=",
                                    "/=",
                                    "%=",
                                    "&=",
                                    "|=",
                                    "^=",
                                    "++",
                                    "--",
                                    ">>",
                                    "<<",
                                    "||",
                                    "&&",
                                    "==",
                                    "!=",
                                    ">=",
                                    "<=",
                                    "->",
                                    ".*",
                                    "::",
                                    ";", /* 1 long */
                                    "<",
                                    ">",
                                    "=",
                                    "+",
                                    "-",
                                    "*",
                                    "/",
                                    "%",
                                    "~",
                                    "&",
                                    "|",
                                    "^",
                                    "!",
                                    "(",
                                    ")",
                                    "[",
                                    "]",
                                    "{",
                                    "}",
                                    ",",
                                    "?",
                                    ":",
                                    ".",
                                    "$"};
      // clang-format on
      BUILD_TOKEN_GUARD()
      int32_t symbolLineNumber = lineNumber;
      int32_t symbolPosition = (int32_t)(head - lineStart);
      for (size_t i = 0; i < ctCStaticArrayLen(knownSymbols); i++) {
         size_t symbolLength = strlen(knownSymbols[i]);
         if (ctCStrNEql(head, knownSymbols[i], symbolLength)) {
            tokens.Append(ctLexerToken(CT_LEXER_TOKEN_SYMBOLS,
                                       symbolLineNumber,
                                       symbolPosition,
                                       head,
                                       head + symbolLength));
            seekHead(&head, &lineStart, &lineNumber, (int)symbolLength - 1);
            break;
         }
      }
   }
   return CT_SUCCESS;
}

void ctLexerToken::GetContents(ctStringUtf8& output, bool append) const {
   if (!append) { output.Clear(); }
   output.Append(pBegin, (size_t)(pEnd - pBegin));
}

void ctLexerToken::GetContents(double& output) const {
   output = ctStringToDouble(pBegin, (size_t)(pEnd - pBegin));
}

void ctLexerToken::GetContents(long double& output) const {
   output = ctStringToLongDouble(pBegin, (size_t)(pEnd - pBegin));
}

void ctLexerToken::GetContents(char& output) const {
   output = *pBegin;
}

bool ctLexerToken::ExpectContents(const char* expected, bool startsWith) const {
   if (!expected) { return true; } /* no preference */
   size_t length = startsWith ? strlen(expected) : SIZE_MAX;
   const size_t contentLength = (size_t)(pEnd - pBegin);
   if (length > contentLength) { length = contentLength; }
   return ctCStrNEql(pBegin, expected, length);
}

ctResults ctLexerIterator::GetDelimeterIterator(ctLexerIterator& iteratorOut,
                                                const char* delimeter,
                                                bool seekEnd) {
   return GetBracketIterator(iteratorOut, NULL, delimeter, seekEnd);
}

ctResults ctLexerIterator::GetBracketIterator(ctLexerIterator& iteratorOut,
                                              const char* open,
                                              const char* close,
                                              bool seekEnd,
                                              bool multiLevel,
                                              bool startSensitive) {
   ctLexerIterator original = SaveState();
   int32_t level = 0;
   const ctLexerToken* pFBegin = NULL;
   const ctLexerToken* pFEnd = NULL;
   if (!open) { pFBegin = pBegin; }
   if (!close) { pFEnd = pEnd; }
   ctLexerIterator it = (*this);
   if (startSensitive) {
      if (it.Token().isSymbol(open)) {
         it++;
         if (!it) { return CT_FAILURE_OUT_OF_BOUNDS; }
         pFBegin = it.TokenPtr();
      } else {
         return CT_FAILURE_NOT_FOUND;
      }
   }
   for (; it; it++) {
      if (!pFBegin && level == 0 && it.Token().isSymbol(open)) {
         ctLexerIterator itcpy = it;
         itcpy++;
         if (!itcpy) { return CT_FAILURE_OUT_OF_BOUNDS; }
         pFBegin = itcpy.TokenPtr();
         if (pFEnd) { break; }
      } else if (!pFEnd && level == 0 && it.Token().isSymbol(close)) {
         ctLexerIterator itcpy = it;
         itcpy--;
         if (!itcpy) { return CT_FAILURE_OUT_OF_BOUNDS; }
         pFEnd = itcpy.TokenPtr();
         if (pFBegin) { break; }
      } else if (multiLevel && (it.Token().isSymbol("{") || it.Token().isSymbol("(") ||
                                it.Token().isSymbol("[") || it.Token().isSymbol("<"))) {
         level++;
      } else if (multiLevel && (it.Token().isSymbol("}") || it.Token().isSymbol(")") ||
                                it.Token().isSymbol("]") || it.Token().isSymbol(">"))) {
         level--;
      }
   }
   if (!pFBegin || !pFEnd) { return CT_FAILURE_NOT_FOUND; }
   it++;         /* skip past delimeter */
   (*this) = it; /* successful iteration progresses */
   iteratorOut = ctLexerIterator(pFBegin, pFBegin, pFEnd);
   if (!seekEnd) { RestoreState(original); }
   return CT_SUCCESS;
}

size_t ctLexerIterator::GetArraySize(const char* separator, int32_t targetLevel) {
   if (BeginPtr() == EndPtr()) { return 0; }
   size_t result = 0;
   int32_t level = 0;
   for (ctLexerIterator it = (*this); it; it++) {
      if (level == targetLevel && it.Token().isSymbol(separator)) {
         result++;
      } else if (it.Token().isSymbol("{") || it.Token().isSymbol("(") ||
                 it.Token().isSymbol("[") || it.Token().isSymbol("<")) {
         level++;
         continue;
      } else if (it.Token().isSymbol("}") || it.Token().isSymbol(")") ||
                 it.Token().isSymbol("]") || it.Token().isSymbol(">")) {
         level--;
         continue;
      }
   }
   return result;
}

ctResults ctLexerIterator::SeekToCommentedLineSubject() {
   if (!(*this)) { return CT_FAILURE_OUT_OF_BOUNDS; }
   if (!Token().isComment()) { return CT_FAILURE_SYNTAX_ERROR; }
   const int32_t lineCommentNumber = Token().GetLineNumber();
   /* check if previous token is on the same line, if so seek back */
   ctLexerIterator seekBack = (*this);
   ctLexerIterator result = (*this);
   seekBack--;
   while (seekBack && seekBack.Token().GetLineNumber() == lineCommentNumber) {
      if (!seekBack.Token().isComment()) { result = seekBack; }
      seekBack--;
   }
   if (result.TokenPtr() != TokenPtr()) {
      (*this) = result;
   } else { /* otherwise seek forward until the next line */
      for (; result; result++) {
         if (!result.Token().isComment() &&
             result.Token().GetLineNumber() != lineCommentNumber) {
            (*this) = result;
            break;
         }
      }
   }
   return CT_SUCCESS;
}

bool ctLexerIterator::CopyIfGreater(ctLexerIterator other) {
   if (!(*this) || !other) { return false; }
   if (TokenPtr() < other.TokenPtr()) {
      (*this) = other;
      return true;
   }
   return false;
}

ctResults ctLexerIterator::ReadAsVariable(ctLexerIterator* pNameOut,
                                          ctLexerIterator* pTypeOut,
                                          ctLexerIterator* pValueItOut,
                                          ctLexerIterator* pArrayInfoOut,
                                          bool seekEnd) {
   /* A variable may look something like this
  static const unsigned long NS::value*&<TEMPLATE, C<32>> variableName[][] = VALUE
  -------------------- type -----------------------------|----name---|arr|-value-*/
   if (!(*this)) { return CT_FAILURE_OUT_OF_BOUNDS; }
   ctLexerIterator original = SaveState();
   const ctLexerToken* pTypeBegin = TokenPtr();
   CT_RETURN_FAIL(ReadAsTypeInfo(NULL, NULL));
   const ctLexerToken* pTypeEnd = TokenPtr() - 1;
   if (pTypeOut) { *pTypeOut = ctLexerIterator(pTypeBegin, pTypeBegin, pTypeEnd); }
   if (pNameOut) { *pNameOut = ctLexerIterator(TokenPtr()); }
   DigestToken(); /* name */
   bool hasArray = false;
   const ctLexerToken* pArrayStart = TokenPtr();
   while (DigestBracketSection("[", "]", false)) {
      hasArray = true;
   }
   const ctLexerToken* pArrayEnd = TokenPtr() - 1;
   if (hasArray && pArrayInfoOut) {
      *pArrayInfoOut = ctLexerIterator(pArrayStart, pArrayStart, pArrayEnd);
   }
   if (pValueItOut) { GetBracketIterator(*pValueItOut, "=", NULL); }
   if (!seekEnd) { RestoreState(original); }
   return CT_SUCCESS;
}

ctResults ctLexerIterator::ReadAsTypeInfo(ctLexerIterator* pTypeNameOut,
                                          ctLexerIterator* pTemplateOut,
                                          size_t* pPtrLevel,
                                          size_t* pRefLevel,
                                          bool* pIsConst,
                                          bool* pIsStatic,
                                          bool seekEnd) {
   /* A type may look something like this
  static const unsigned long NS::value<TEMPLATE, C<32>>*&
  static|const|-------type-----------|---template----|p|r*/
   if (!(*this)) { return CT_FAILURE_OUT_OF_BOUNDS; }
   ctLexerIterator original = SaveState();
   if (DigestToken("static") && pIsStatic) { *pIsStatic = true; }
   if (DigestToken("const") && pIsConst) { *pIsConst = true; }

   const ctLexerToken* pTypeBegin = TokenPtr();
   DigestCPPTypeName();
   const ctLexerToken* pTypeEnd = TokenPtr();
   if (pTypeNameOut) {
      *pTypeNameOut = ctLexerIterator(pTypeBegin, pTypeBegin, pTypeEnd);
   }

   const ctLexerToken* pTemplateBegin = TokenPtr();
   bool hasTemplate = DigestBracketSection("<", ">");
   const ctLexerToken* pTemplateEnd = TokenPtr();
   if (hasTemplate && pTemplateOut) {
      *pTemplateOut = ctLexerIterator(pTemplateBegin, pTemplateBegin, pTemplateEnd);
   }

   if (pPtrLevel) { *pPtrLevel = 0; }
   while (DigestToken("*")) {
      if (pPtrLevel) { (*pPtrLevel)++; }
   }
   if (pRefLevel) { *pRefLevel = 0; }
   while (DigestToken("&")) {
      if (pRefLevel) { (*pRefLevel)++; }
   }
   if (!seekEnd) { RestoreState(original); }
   return CT_SUCCESS;
}

ctResults ctLexerIterator::ReadAsArrayInfo(int32_t& pSizeOut, bool seekEnd) {
   if (!(*this)) { return CT_FAILURE_OUT_OF_BOUNDS; }
   ctLexerIterator original = SaveState();
   ctLexerIterator bracketIt;
   CT_RETURN_FAIL(GetBracketIterator(bracketIt, "[", "]", seekEnd));
   CT_RETURN_FAIL(bracketIt.ReadValue(pSizeOut));
   if (!seekEnd) { RestoreState(original); }
   return CT_SUCCESS;
}

ctResults ctLexerIterator::ReadAsEnumEntry(ctLexerIterator* pNameOut,
                                           ctLexerIterator* pAssignmentOut,
                                           bool seekEnd) {
   /* An enum entry may look something like this
   MY_ENUM_VALUE = NS::MY_OTHER VALUE | 32
   -----name-----|-----assignment---------*/
   if (!(*this)) { return CT_FAILURE_OUT_OF_BOUNDS; }
   ctLexerIterator original = SaveState();
   if (pNameOut) {
      *pNameOut = TokenPtr();
      (*this)++;
   }
   if (pAssignmentOut) {
      if (GetBracketIterator(*pAssignmentOut, "=", ",") != CT_SUCCESS) {
         GetBracketIterator(*pAssignmentOut, "=", NULL); /* end of list */
      }
   }
   if (!seekEnd) { RestoreState(original); }
   return CT_SUCCESS;
}

ctResults ctLexerIterator::ReadAsEnumDefinition(ctLexerIterator* pNameOut,
                                                bool* pIsClass,
                                                bool seekEnd) {
   /* An enum definition may look something like this
   enum myenum
   skip|class|-name-*/
   if (!(*this)) { return CT_FAILURE_OUT_OF_BOUNDS; }
   ctLexerIterator original = SaveState();
   DigestToken("enum");

   if (DigestToken("class") || DigestToken("struct")) {
      if (pIsClass) { *pIsClass = true; }
   } else if (pIsClass) {
      *pIsClass = false;
   }
   if (pNameOut) { *pNameOut = TokenPtr(); }
   DigestToken();
   if (!seekEnd) { RestoreState(original); }
   return CT_SUCCESS;
}

ctResults ctLexerIterator::ReadAsStructDefinition(ctLexerIterator* pNameOut,
                                                  bool seekEnd) {
   /* An struct definition may look something like this
   class myclass : public NS::otherclass {
   skip|--name--*/
   if (!(*this)) { return CT_FAILURE_OUT_OF_BOUNDS; }
   ctLexerIterator original = SaveState();
   DigestToken("struct");
   DigestToken("class");
   DigestToken("CT_API");
   if (pNameOut) { *pNameOut = TokenPtr(); }
   DigestToken();
   if (!seekEnd) { RestoreState(original); }
   return CT_SUCCESS;
}

ctResults ctLexerIterator::ReadAsStructInheritanceInfo(ctLexerIterator* pTypeNameOut,
                                                       ctLexerIterator* pScopeOut,
                                                       bool seekEnd) {
   /* Inheritance info may look something like this
   , public MyNamespace::Class
   X|scope-|----typeinfo----- */
   if (!(*this)) { return CT_FAILURE_OUT_OF_BOUNDS; }
   ctLexerIterator original = SaveState();
   if (Token().isSymbol("{") || Token().isSymbol(";")) {
      return CT_FAILURE_END_OF_STREAM;
   }
   DigestToken(":");
   DigestToken(",");
   ctLexerIterator tmpScope = TokenPtr();
   if (DigestToken("public") || DigestToken("protected") || DigestToken("private")) {
      if (pScopeOut) { *pScopeOut = tmpScope; }
   }

   const ctLexerToken* pTypeBegin = TokenPtr();
   DigestCPPTypeName();
   const ctLexerToken* pTypeEnd = TokenPtr() - 1;
   if (pTypeNameOut) {
      *pTypeNameOut = ctLexerIterator(pTypeBegin, pTypeBegin, pTypeEnd);
   }
   if (!seekEnd) { RestoreState(original); }
   return CT_SUCCESS;
}

ctStringUtf8 ctLexerIterator::ToString() {
   ctStringUtf8 result = "";
   ctLexerIterator it = (*this);
   bool lastWasIdent = false;
   for (ctLexerIterator it = (*this); it; it++) {
      if (lastWasIdent && it.Token().isIdentifier()) { result += " "; }
      lastWasIdent = it.Token().isIdentifier();
      it.Token().GetContents(result, true);
   }
   return result;
}

ctResults ctLexerIterator::ExpectToken(const char* identifier, bool seekEnd) {
   if (!(*this)) { return CT_FAILURE_OUT_OF_BOUNDS; }
   if (Token().ExpectContents(identifier)) {
      if (seekEnd) { (*this)++; }
      return CT_SUCCESS;
   }
   return CT_FAILURE_SYNTAX_ERROR;
}

bool ctLexerIterator::DigestToken(const char* contents) {
   return ExpectToken(contents) == CT_SUCCESS;
}

bool ctLexerIterator::DigestBracketSection(const char* open,
                                           const char* close,
                                           bool multiLevel) {
   ctLexerIterator tmp;
   return GetBracketIterator(tmp, open, close, true, multiLevel, true) == CT_SUCCESS;
}

void ctLexerIterator::DigestCPPTypeName() {
   DigestToken("CT_API");
   DigestToken("enum");
   DigestToken("class");
   DigestToken("struct");

   DigestToken("unsigned");
   DigestToken("signed");

   /* handle the "long" combo issue */
   if (Token().isIdentifier("long")) {
      DigestToken("char");
      DigestToken("short");
      DigestToken("int");
      DigestToken("long");
      DigestToken("float");
      DigestToken("double");
   } else { /* normal name */
      bool hadNamespace = false;
      do {
         DigestToken(); /* name */
         if (DigestToken("::")) {
            hadNamespace = true;
         } else {
            hadNamespace = false;
         }
      } while (hadNamespace);
   }
}

void ctLexerIterator::NextArrayObject() {
   DigestToken(",");
}

ctResults ctLexerIterator::ReadValue(bool& out, bool seekEnd) {
   if (!(*this)) { return CT_FAILURE_OUT_OF_BOUNDS; }
   if (Token().isIdentifier("true")) {
      out = true;
   } else if (Token().isIdentifier("false")) {
      out = false;
   } else if (Token().isNumber()) {
      double tmp;
      CT_RETURN_FAIL(ReadValue(tmp, false));
      out = (bool)tmp;
   }
   if (seekEnd) {
      (*this)++;
      NextArrayObject();
   }
   return CT_SUCCESS;
}

ctResults ctLexerIterator::ReadValue(uint8_t& out, bool seekEnd) {
   double tmp;
   CT_RETURN_FAIL(ReadValue(tmp, seekEnd));
   out = (uint8_t)tmp;
   return CT_SUCCESS;
}

ctResults ctLexerIterator::ReadValue(int8_t& out, bool seekEnd) {
   double tmp;
   CT_RETURN_FAIL(ReadValue(tmp, seekEnd));
   out = (int8_t)tmp;
   return CT_SUCCESS;
}

ctResults ctLexerIterator::ReadValue(uint16_t& out, bool seekEnd) {
   double tmp;
   CT_RETURN_FAIL(ReadValue(tmp, seekEnd));
   out = (uint16_t)tmp;
   return CT_SUCCESS;
}

ctResults ctLexerIterator::ReadValue(int16_t& out, bool seekEnd) {
   double tmp;
   CT_RETURN_FAIL(ReadValue(tmp, seekEnd));
   out = (int16_t)tmp;
   return CT_SUCCESS;
}

ctResults ctLexerIterator::ReadValue(uint32_t& out, bool seekEnd) {
   double tmp;
   CT_RETURN_FAIL(ReadValue(tmp, seekEnd));
   out = (uint32_t)tmp;
   return CT_SUCCESS;
}

ctResults ctLexerIterator::ReadValue(int32_t& out, bool seekEnd) {
   double tmp;
   CT_RETURN_FAIL(ReadValue(tmp, seekEnd));
   out = (int32_t)tmp;
   return CT_SUCCESS;
}

ctResults ctLexerIterator::ReadValue(uint64_t& out, bool seekEnd) {
   long double tmp;
   CT_RETURN_FAIL(ReadValue(tmp, seekEnd));
   out = (uint64_t)tmp;
   return CT_SUCCESS;
}

ctResults ctLexerIterator::ReadValue(int64_t& out, bool seekEnd) {
   long double tmp;
   CT_RETURN_FAIL(ReadValue(tmp, seekEnd));
   out = (int64_t)tmp;
   return CT_SUCCESS;
}

ctResults ctLexerIterator::ReadValue(float& out, bool seekEnd) {
   double tmp;
   CT_RETURN_FAIL(ReadValue(tmp, seekEnd));
   out = (float)tmp;
   return CT_SUCCESS;
}

ctResults ctLexerIterator::ReadValue(double& out, bool seekEnd) {
   if (!(*this)) { return CT_FAILURE_OUT_OF_BOUNDS; }
   if (!Token().isNumber()) { return CT_FAILURE_SYNTAX_ERROR; }
   Token().GetContents(out);
   if (seekEnd) {
      (*this)++;
      NextArrayObject();
   }
   return CT_SUCCESS;
}

ctResults ctLexerIterator::ReadValue(long double& out, bool seekEnd) {
   if (!(*this)) { return CT_FAILURE_OUT_OF_BOUNDS; }
   if (!Token().isNumber()) { return CT_FAILURE_SYNTAX_ERROR; }
   Token().GetContents(out);
   if (seekEnd) {
      (*this)++;
      NextArrayObject();
   }
   return CT_SUCCESS;
}

ctResults ctLexerIterator::ReadValue(char& out, bool seekEnd) {
   if (!(*this)) { return CT_FAILURE_OUT_OF_BOUNDS; }
   if (!Token().isType(CT_LEXER_TOKEN_CHAR)) { return CT_FAILURE_SYNTAX_ERROR; }
   Token().GetContents(out);
   if (seekEnd) {
      (*this)++;
      NextArrayObject();
   }
   return CT_SUCCESS;
}

ctResults ctLexerIterator::ReadValue(ctStringUtf8& out, bool seekEnd) {
   if (!(*this)) { return CT_FAILURE_OUT_OF_BOUNDS; }
   if (!Token().isString()) { return CT_FAILURE_SYNTAX_ERROR; }
   Token().GetContents(out);
   out.ProcessEscapeCodes();
   if (seekEnd) {
      (*this)++;
      NextArrayObject();
   }
   return CT_SUCCESS;
}

ctResults ctLexerIterator::ReadValue(ctVec2& out, bool seekEnd) {
   ctLexerIterator it;
   CT_RETURN_FAIL(GetBracketIterator(it, "{", "}", seekEnd));
   CT_RETURN_FAIL(it.ReadValue(out.x));
   CT_RETURN_FAIL(it.ReadValue(out.y));
   if (seekEnd) { NextArrayObject(); }
   return CT_SUCCESS;
}

ctResults ctLexerIterator::ReadValue(ctVec3& out, bool seekEnd) {
   ctLexerIterator it;
   CT_RETURN_FAIL(GetBracketIterator(it, "{", "}", seekEnd));
   CT_RETURN_FAIL(it.ReadValue(out.x));
   CT_RETURN_FAIL(it.ReadValue(out.y));
   CT_RETURN_FAIL(it.ReadValue(out.z));
   if (seekEnd) { NextArrayObject(); }
   return CT_SUCCESS;
}

ctResults ctLexerIterator::ReadValue(ctVec4& out, bool seekEnd) {
   ctLexerIterator it;
   CT_RETURN_FAIL(GetBracketIterator(it, "{", "}", seekEnd));
   CT_RETURN_FAIL(it.ReadValue(out.x));
   CT_RETURN_FAIL(it.ReadValue(out.y));
   CT_RETURN_FAIL(it.ReadValue(out.z));
   CT_RETURN_FAIL(it.ReadValue(out.w));
   if (seekEnd) { NextArrayObject(); }
   return CT_SUCCESS;
}

ctResults ctLexerIterator::ReadValue(ctMat4& out, bool seekEnd) {
   ctLexerIterator it;
   CT_RETURN_FAIL(GetBracketIterator(it, "{", "}", seekEnd));
   for (size_t i = 0; i < 4; i++) {
      CT_RETURN_FAIL(it.ReadValue(*(ctVec4*)out.data[i]));
      it.NextArrayObject();
   }
   if (seekEnd) { NextArrayObject(); }
   return CT_SUCCESS;
}

ctResults ctLexerIterator::ReadValue(ctGUID& out, bool seekEnd) {
   ctStringUtf8 hexStr;
   CT_RETURN_FAIL(ReadValue(hexStr, seekEnd));
   size_t byteCount = hexStr.ByteLength() / 2;
   if (byteCount < 16) { return CT_FAILURE_OUT_OF_BOUNDS; }
   out = ctGUID(hexStr.CStr());
   return CT_SUCCESS;
}

ctResults ctLexerIterator::ReadValue(ctDynamicArray<uint8_t>& out, bool seekEnd) {
   ctStringUtf8 hexStr;
   CT_RETURN_FAIL(ReadValue(hexStr, seekEnd));
   size_t byteCount = hexStr.ByteLength() / 2;
   CT_RETURN_FAIL(out.Resize(byteCount));
   ctHexToBytes(byteCount, hexStr.CStr(), out.Data());
   return CT_SUCCESS;
}
