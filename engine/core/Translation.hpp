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
   CT_TRANSLATION_CATAGORY_CORE, /* Engine strings */
   CT_TRANSLATION_CATAGORY_APP,  /* Application specific strings */
   CT_TRANSLATION_CATAGORY_GAME, /* Game strings */
   CT_TRANSLATION_CATAGORY_BANK, /* Banked strings (ex: mission specific) */
   CT_TRANSLATION_CATAGORY_COUNT,
};

/* Get a translated string
WARNING! NOT GUARANTEED TO BE LONG TERM STORAGE! ONLY TRUST FOR ONE FRAME!
Character maps can change when language is set, do not cache! */
const char* ctGetLocalString(ctTranslationCatagory category,
                             const char* nativeText);

#define CT_NC(_text_) ctGetLocalString(CT_TRANSLATION_CATAGORY_CORE, _text_)
#define CT_NA(_text_) ctGetLocalString(CT_TRANSLATION_CATAGORY_APP, _text_)
#define CT_NG(_text_) ctGetLocalString(CT_TRANSLATION_CATAGORY_GAME, _text_)
#define CT_NB(_text_) ctGetLocalString(CT_TRANSLATION_CATAGORY_BANK, _text_)

class ctTranslation : public ctModuleBase {
public:
   ctTranslation(bool shared);

   ctResults Startup() final;
   ctResults Shutdown() final;

   ctResults SetDictionary(ctTranslationCatagory category,
                           const char* basePath);
   ctResults LoadLanguage(const char* languageName);
   ctResults LoadDictionary(ctTranslationCatagory category);
   ctResults LoadAll();

   const char* GetLocalString(ctTranslationCatagory category,
                              const char* nativeText) const;

private:
   /* Returns the users preferred language if possible
    Ideally in RFC 4646 but the OS might have other plans
    Search key-words to find the closest known language or
    skip language auto-detect if data is in unknown format */
   ctStringUtf8 GetLocalLanguage() const;

   class _dictionary {
   public:
      ctHashTable<const char*, uint64_t> strings;
      ctStringUtf8 basePath;
   };
   ctStringUtf8 language;
   ctStaticArray<_dictionary, CT_TRANSLATION_CATAGORY_COUNT> dictionaries;
};