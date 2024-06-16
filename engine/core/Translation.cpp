/*
   Copyright 2022 MacKenzie Strand

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
#include "FileSystem.hpp"
#include "resource/JSONResource.hpp"
#include "resource/TranslationResource.hpp"
#include "Settings.hpp"
#include "formats/mo/MO.h"
#include <locale.h>

ctTranslation* gMainTranslationSystem = NULL;

ctTranslation::ctTranslation(bool shared) {
   if (shared) { gMainTranslationSystem = this; }
   for (int i = 0; i < CT_TRANSLATION_CATAGORY_COUNT; i++) {
      dictionaries.Append(new _dictionary());
   }
}

void _setLanguageCb(const char* val, void* data) {
   ctTranslation* pTranslation = (ctTranslation*)data;
   pTranslation->LoadLanguage(val);
}

#include "system/System.h"

ctResults ctTranslation::Startup() {
   ZoneScoped;
   char languageNameBuff[32];
   memset(languageNameBuff, 0, 32);
   ctSystemInitialGetLanguage(languageNameBuff, 32);
   isoLanguage = languageNameBuff;
   setlocale(LC_ALL, "C"); /* Unify C Locale */
   fullLanguageName = "?";
   ctDebugLog("OS Reported Language: %s", isoLanguage.CStr());

   ctSettingsSection* settings = Engine->Settings->CreateSection("Translation", 1);
   settings->BindString(&isoLanguage,
                        true,
                        true,
                        "Language",
                        "Code for the language to use in RFC 4646 format.");

   SetDictionary(CT_TRANSLATION_CATAGORY_CORE, "core");
   LoadLanguage(isoLanguage.CStr());
   return CT_SUCCESS;
}

ctResults ctTranslation::Shutdown() {
   for (size_t i = 0; i < dictionaries.Count(); i++) {
      if (dictionaries[i]) { delete dictionaries[i]; }
   }
   return CT_SUCCESS;
}

const char* ctTranslation::GetModuleName() {
   return "Translation";
}

ctResults ctTranslation::SetDictionary(ctTranslationCatagory category,
                                       const char* basePath) {
   _dictionary& dict = *dictionaries[category];
   dict.basePath = basePath;
   return CT_SUCCESS;
}

bool languageCompare(const char* a, const char* b) {
   const char* in = b;
   if (!a || !b) { return false; }
   while (*a && *b && (*a == *b || *b == '*')) {
      a++;
      b++;
      if (!*b) { return true; }
   }
   return false;
}

ctResults ctTranslation::LoadLanguage(const char* isoCode) {
   /* Find language file */
   {
      ctFile file;
      ctHandlePtr<ctResourceJSON> json =
        ctGetResourceCritical(ctResourceJSON, "LANGUAGES");
      ctJSONReadEntry languagesJson = ctJSONReadEntry();
      json.Get().GetRootEntry(languagesJson);
      int languageEntryCount = languagesJson.GetObjectEntryCount();
      bool found = false;
      for (int i = 0; i < languageEntryCount; i++) {
         ctJSONReadEntry entry = ctJSONReadEntry();
         ctStringUtf8 name = ctStringUtf8();
         languagesJson.GetObjectEntry(i, entry, &name);
         if (name == "DEFAULT") { entry.GetString(fullLanguageName); }
         if (languageCompare(isoCode, name.CStr())) {
            entry.GetString(fullLanguageName);
            found = true;
         }
      }
      if (found) {
         isoLanguage = isoCode;
      } else {
         ctDebugWarning("Could not find language: %s", isoCode);
      }
   }
   LoadAll();
   return CT_SUCCESS;
}

ctResults ctTranslation::LoadDictionary(ctTranslationCatagory category) {
   ZoneScoped;
   /* Load strings */
   {
      ctStringUtf8 path;
      path.Printf(256,
                  "text-%s-%s",
                  dictionaries[category]->basePath.CStr(),
                  fullLanguageName.ToLower().CStr());
      dictionaries[category]->translation =
        ctGetResourceCritical(ctResourceTranslation, path.CStr());
   }
   return CT_SUCCESS;
}

ctResults ctTranslation::LoadAll() {
   ZoneScoped;
   LoadDictionary(CT_TRANSLATION_CATAGORY_CORE);
   return CT_SUCCESS;
}

ctStringUtf8 ctTranslation::GetISOLanguage() const {
   return isoLanguage;
}

ctStringUtf8 ctTranslation::GetCurrentLanguage() const {
   return fullLanguageName;
}

const char* ctTranslation::GetLocalString(ctTranslationCatagory category,
                                          const char* tag,
                                          const char* nativeText) const {
   ZoneScoped;
   if (!isStarted()) { return nativeText; }
   if (!dictionaries[category]->translation.isHandleValid()) { return nativeText; }
   ctResourceTranslation* pTrans = dictionaries[category]->translation.GetPtr();
   const char*
     translation = /* todo: fix handles getting scrambled (pTrans ends up a Shader) */
       pTrans->FindTranslation(tag);
   if (!translation) { return nativeText; }
   return translation;
}

const char* ctGetLocalString(ctTranslationCatagory category,
                             const char* tag,
                             const char* nativeText) {
   if (gMainTranslationSystem) {
      return gMainTranslationSystem->GetLocalString(category, tag, nativeText);
   }
   return nativeText;
}