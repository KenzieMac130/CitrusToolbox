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
#include "utilities/BloomFilter.hpp"
#include "ModuleBase.hpp"

#if CITRUS_INCLUDE_AUDITION
#include "audition/HotReloadDetection.hpp"
#endif

enum ctTranslationCatagory {
   CT_TRANSLATION_CATAGORY_CORE = 0, /* Engine strings */
   CT_TRANSLATION_CATAGORY_APP = 0,  /* Application specific strings */
   CT_TRANSLATION_CATAGORY_GAME = 0, /* Game strings */
   CT_TRANSLATION_CATAGORY_BANK0,    /* Banked strings (ex: mission specific) */
   CT_TRANSLATION_CATAGORY_BANK1,    /* Banked strings (ex: level specific) */
   CT_TRANSLATION_CATAGORY_BANK2,    /* Banked strings (ex: story progress) */
   CT_TRANSLATION_CATAGORY_COUNT,
};

/* Get a translated string
WARNING! NOT GUARANTEED TO BE LONG TERM STORAGE! ONLY TRUST FOR ONE FRAME!
String maps can change when language is set. DO NOT CACHE POINTER! */
CT_API const char*
ctGetLocalString(ctTranslationCatagory category, const char* tag, const char* nativeText);

/* Translate core string */
#define CT_NCT(_tag, _txt) ctGetLocalString(CT_TRANSLATION_CATAGORY_CORE, _tag, _txt)
#define CT_NC(_txt)        CT_NCT(_txt, _txt)
/* Translate application string */
#define CT_NAT(_tag, _txt) ctGetLocalString(CT_TRANSLATION_CATAGORY_APP, _tag, _txt)
#define CT_NA(_txt)        CT_NAT(_txt, _txt)
/* Translate game string */
#define CT_NGT(_tag, _txt) ctGetLocalString(CT_TRANSLATION_CATAGORY_GAME, _tag, _txt)
#define CT_NG(_txt)        CT_NGT(_txt, _txt)
/* Translate string from bank 0 (tip: make the native string a generic identifier) */
#define CT_NB0T(_tag, _txt) ctGetLocalString(CT_TRANSLATION_CATAGORY_BANK0, _tag, _txt)
#define CT_NB0(_txt)        CT_NB0T(_txt, _txt)
/* Translate string from bank 1 (tip: make the native string a generic identifier) */
#define CT_NB1T(_tag, _txt) ctGetLocalString(CT_TRANSLATION_CATAGORY_BANK1, _tag, _txt)
#define CT_NB1(_txt)        CT_NB1T(_txt, _txt)
/* Translate string from bank 2 (tip: make the native string a generic identifier) */
#define CT_NB2T(_tag, _txt) ctGetLocalString(CT_TRANSLATION_CATAGORY_BANK2, _tag, _txt)
#define CT_NB2(_txt)        CT_NB2T(_txt, _txt)

class CT_API ctTranslation : public ctModuleBase {
public:
   ctTranslation(bool shared);

   ctResults Startup() final;
   ctResults Shutdown() final;

   ctResults NextFrame();

   ctResults SetDictionary(ctTranslationCatagory category, const char* basePath);
   ctResults LoadLanguage(const char* isoCode);
   ctResults LoadDictionary(ctTranslationCatagory category);
   ctResults LoadAll();

   const char* GetLocalString(ctTranslationCatagory category,
                              const char* tag,
                              const char* nativeText) const;

   /* Returns the users preferred language if possible
    Ideally in RFC 4646 but the OS might have other plans
    Search key-words to find the closest known language or
    skip language auto-detect if data is in unknown format.
    Modify this function when porting to other platforms.
    Do not rely on this, always provide language selection.*/
   ctStringUtf8 GetISOLanguage() const;
   ctStringUtf8 GetCurrentLanguage() const;

private:
#if CITRUS_INCLUDE_AUDITION
   ctHotReloadCategory TextHotReload;
#endif

   class _dictionary {
   public:
      ctBloomFilter<uint64_t, 1024, 4> bloom;
      ctHashTable<ctStringUtf8, uint64_t> strings;
      ctStringUtf8 basePath;
      /* Used for dumping strings */
      ctHashTable<ctStringUtf8, uint64_t> nativestrings;
   };
   ctStringUtf8 isoLanguage;
   ctStringUtf8 fullLanguageName;
   ctStaticArray<_dictionary*, CT_TRANSLATION_CATAGORY_COUNT> dictionaries;
};