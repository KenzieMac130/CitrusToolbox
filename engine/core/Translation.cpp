#include "Translation.hpp"
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

#include "Translation.hpp"
#include "EngineCore.hpp"

#ifdef _WIN32
#include <Windows.h>
#else
#include <locale.h>
#endif

ctTranslation* mainTranslationSystem;

ctTranslation::ctTranslation(bool shared) {
   if (shared) { mainTranslationSystem = this; }
   for (int i = 0; i < CT_TRANSLATION_CATAGORY_COUNT; i++) {
       dictionaries.Append(ctTranslation::_dictionary());
   }
}

ctResults ctTranslation::Startup() {
   ZoneScoped;
#if defined(_WIN32)
   wchar_t data[LOCALE_NAME_MAX_LENGTH];
   GetLocaleInfoEx(
     LOCALE_NAME_USER_DEFAULT, LOCALE_SNAME, data, LOCALE_NAME_MAX_LENGTH);
   language = ctStringUtf8(data);
#elif defined(__linux__)
   /* Try to extract a similar string off "setlocale" (tested only on Ubuntu) */
   const char* cbuf = setlocale(LC_ALL, "");
   size_t max = strlen(cbuf) > 255 ? 255 : strlen(cbuf);
   char scratch[256];
   memset(scratch, 0, 256);
   strncpy(scratch, cbuf, max);
   char* nextVal = scratch;
   while (*nextVal != '\0') {
      if (*nextVal == '_') { *nextVal = '-'; }
      if (*nextVal == '.') {
         *nextVal = '\0';
         break;
      }
      nextVal++;
   }
   result = scratch;
   if (result == "C") { result = "DEFAULT"; }
#else
   language = "DEFAULT";
#endif
   setlocale(LC_ALL, "C"); /* Unify C Locale */
   ctDebugLog("Detected Language: %s", language.CStr());
   return CT_SUCCESS;
}

ctResults ctTranslation::Shutdown() {
   return CT_SUCCESS;
}

ctResults ctTranslation::SetDictionary(ctTranslationCatagory category,
                                       const char* basePath) {
   _dictionary& dict = dictionaries[category];
   dict.basePath = basePath;
   dict.strings = ctHashTable<const char*, uint64_t>();
   return CT_SUCCESS;
}

ctResults ctTranslation::LoadLanguage(const char* languageName) {
   language = languageName;
   LoadAll();
   return CT_SUCCESS;
}

ctResults ctTranslation::LoadDictionary(ctTranslationCatagory category) {
   ZoneScoped;
   /* Todo: Load from JSON */
   /* dict.strings = ctHashTable<const char*, uint64_t>(count * 1.5);
   for (int i = 0; i < count; i++) {
       const char* str = nativeTexts[i];
       const uint64_t hash = XXH64(str, strlen(str), 0);
       dict.strings.Insert(hash, localTexts[i]);
   }*/
   return CT_SUCCESS;
}

ctResults ctTranslation::LoadAll() {
   ZoneScoped;
   LoadDictionary(CT_TRANSLATION_CATAGORY_CORE);
   LoadDictionary(CT_TRANSLATION_CATAGORY_APP);
   LoadDictionary(CT_TRANSLATION_CATAGORY_GAME);
   LoadDictionary(CT_TRANSLATION_CATAGORY_BANK0);
   LoadDictionary(CT_TRANSLATION_CATAGORY_BANK1);
   LoadDictionary(CT_TRANSLATION_CATAGORY_BANK2);
   return CT_SUCCESS;
}

ctStringUtf8 ctTranslation::GetUserOSLanguage() {
   return language;
}

const char* ctTranslation::GetLocalString(ctTranslationCatagory category,
                                          const char* nativeText) const {
   ZoneScoped;
   if (!isStarted()) { return nativeText; }
   uint64_t hash = XXH64(nativeText, strlen(nativeText), 0);
   /* Todo: use bloom filter to detect if translation exists */
   const char** localText = dictionaries[category].strings.FindPtr(hash);
   if (localText) { return *localText; }
   return nativeText;
}

const char* ctGetLocalString(ctTranslationCatagory category,
                             const char* nativeText) {
   if (mainTranslationSystem) {
      mainTranslationSystem->GetLocalString(category, nativeText);
   }
   return nativeText;
}
