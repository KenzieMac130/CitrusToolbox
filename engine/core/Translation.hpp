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
   CT_TRANSLATION_CATAGORY_BANK0, /* Banked strings (ex: mission specific) */
   CT_TRANSLATION_CATAGORY_BANK1, /* Banked strings (ex: level specific) */
   CT_TRANSLATION_CATAGORY_BANK2, /* Banked strings (ex: story progress) */
   CT_TRANSLATION_CATAGORY_COUNT,
};

/* Get a translated string
WARNING! NOT GUARANTEED TO BE LONG TERM STORAGE! ONLY TRUST FOR ONE FRAME!
String maps can change when language is set. DO NOT CACHE POINTER! */
CT_API const char* ctGetLocalString(ctTranslationCatagory category,
                             const char* nativeText);

/* Translate core string */
#define CT_NC(_text_) ctGetLocalString(CT_TRANSLATION_CATAGORY_CORE, _text_)
/* Translate application string */
#define CT_NA(_text_) ctGetLocalString(CT_TRANSLATION_CATAGORY_APP, _text_)
/* Translate game string */
#define CT_NG(_text_) ctGetLocalString(CT_TRANSLATION_CATAGORY_GAME, _text_)
/* Translate string from bank 0 (tip: make the native string a generic identifier) */
#define CT_NB0(_text_) ctGetLocalString(CT_TRANSLATION_CATAGORY_BANK0, _text_)
/* Translate string from bank 1 (tip: make the native string a generic identifier) */
#define CT_NB1(_text_) ctGetLocalString(CT_TRANSLATION_CATAGORY_BANK1, _text_)
/* Translate string from bank 2 (tip: make the native string a generic identifier) */
#define CT_NB2(_text_) ctGetLocalString(CT_TRANSLATION_CATAGORY_BANK2, _text_)

class CT_API ctTranslation : public ctModuleBase {
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

   /* Returns the users preferred language if possible
    Ideally in RFC 4646 but the OS might have other plans
    Search key-words to find the closest known language or
    skip language auto-detect if data is in unknown format.
    Modify this function when porting to other platforms.
    Do not rely on this, always provide language selection.*/
   static ctStringUtf8 GetUserOSLanguage();

private:
   class _dictionary {
   public:
      ctHashTable<const char*, uint64_t> strings;
      ctStringUtf8 basePath;
   };
   ctStringUtf8 language;
   ctStaticArray<_dictionary, CT_TRANSLATION_CATAGORY_COUNT> dictionaries;
};