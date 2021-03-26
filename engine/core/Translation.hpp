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

#pragma once

#include "utilities/Common.h"
#include "ModuleBase.hpp"

enum ctTranslationCatagory {
   CT_TRANSLATION_CATAGORY_CORE,
   CT_TRANSLATION_CATAGORY_GAME,
   CT_TRANSLATION_CATAGORY_COUNT,
};

const char* ctGetLocalString(ctTranslationCatagory category,
                             const char* nativeText);
#define CT_NC(_text_) ctGetLocalString(CT_TRANSLATION_CATAGORY_CORE, _text_)
#define CT_NG(_text_) ctGetLocalString(CT_TRANSLATION_CATAGORY_GAME, _text_)

class ctTranslation : public ctModuleBase {
public:
   ctTranslation(const char* nativeLanguage, bool shared);

   ctResults Startup() final;
   ctResults Shutdown() final;

   ctResults SetDictionary(ctTranslationCatagory category,
                           size_t count,
                           const char** nativeTexts);
   ctResults SetLanguage(const char* languageName);
   ctResults LoadDictionary(ctTranslationCatagory category);
   ctResults LoadAll();

   const char* ctGetLocalString(ctTranslationCatagory category,
                                const char* nativeText);

private:
   class _dictionary {
   public:
      ctHashTable<const char*, uint64_t> strings;
   };
   ctStringUtf8 language;
   ctStaticArray<_dictionary, CT_TRANSLATION_CATAGORY_COUNT> dictionaries;
};