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

ctTranslation* mainTranslationSystem;

ctTranslation::ctTranslation(const char* nativeLanguage, bool shared) {
   language = nativeLanguage;
   if (shared) { mainTranslationSystem = this; }
}

ctResults ctTranslation::Startup() {
   return CT_SUCCESS;
}

ctResults ctTranslation::Shutdown() {
   return CT_SUCCESS;
}

ctResults ctTranslation::SetDictionary(ctTranslationCatagory category,
                                       size_t count,
                                       const char** nativeTexts) {
   _dictionary& dict = dictionaries[category];
   dict.strings = ctHashTable<const char*, uint64_t>(count * 1.5);
   for (int i = 0; i < count; i++) {
      const char* str = nativeTexts[i];
      const uint64_t hash = XXH64(str, strlen(str), 0);
      dict.strings.Insert(hash, str);
   }
   SetLanguage(language.CStr());
   return CT_SUCCESS;
}

ctResults ctTranslation::SetLanguage(const char* languageName) {
   language = languageName;
   LoadAll();
   return CT_SUCCESS;
}

ctResults ctTranslation::LoadDictionary(ctTranslationCatagory category) {
   /* Todo: Load from CSV */
   return CT_SUCCESS;
}

ctResults ctTranslation::LoadAll() {
   LoadDictionary(CT_TRANSLATION_CATAGORY_CORE);
   LoadDictionary(CT_TRANSLATION_CATAGORY_GAME);
   return CT_SUCCESS;
}

const char* ctTranslation::ctGetLocalString(ctTranslationCatagory category,
                                            const char* nativeText) {
   uint64_t hash = XXH64(nativeText, strlen(nativeText), 0);
   const char** localText = dictionaries[category].strings.FindPtr(hash);
   if (localText) { return *localText; }
   return nativeText;
}

const char* ctGetLocalString(ctTranslationCatagory category,
                             const char* nativeText) {
   if (mainTranslationSystem) {
      mainTranslationSystem->ctGetLocalString(category, nativeText);
   }
   return nativeText;
}
