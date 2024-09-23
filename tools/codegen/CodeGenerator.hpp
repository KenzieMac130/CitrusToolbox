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

#include "utilities/Common.h"
#include "utilities/Lexer.hpp"

class CT_API ctCodeGenerator {
public:
   ctResults Execute(ctFile& inHeader, ctFile& outHeader, ctFile& outImplementation);

private:
   ctResults ParseCtReflectEnum(ctLexerIterator& it);
   ctResults ParseCtReflectStruct(ctLexerIterator& it);
   ctLexer lexer;

   /* outputs */
   ctHashTable<bool, uint32_t> classForwards;
   ctStringUtf8 headerResult;
   ctStringUtf8 implForwards;
   ctStringUtf8 implBottom;
};