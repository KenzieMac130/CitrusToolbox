#include "Translation.hpp"
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
#if CITRUS_INCLUDE_AUDITION
   Engine->HotReload->RegisterDataCategory(&TextHotReload);
#endif
   char languageNameBuff[32];
   memset(languageNameBuff, 0, 32);
   ctSystemInitialGetLanguage(languageNameBuff, 32);
   isoLanguage = languageNameBuff;
   setlocale(LC_ALL, "C"); /* Unify C Locale */
   fullLanguageName = "?";
   ctDebugLog("OS Reported Language: %s", isoLanguage.CStr());

   ctSettingsSection* settings = Engine->Settings->CreateSection("Translation", 1);
   settings->BindString(
     &isoLanguage, true, true, "Language", "Code for the language to use in RFC 4646 format.");

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

ctResults ctTranslation::NextFrame() {
#if CITRUS_INCLUDE_AUDITION
   if (TextHotReload.isContentUpdated()) {
      LoadAll();
      TextHotReload.ClearChanges();
   }
#endif
   return CT_SUCCESS;
}

ctResults ctTranslation::SetDictionary(ctTranslationCatagory category, const char* basePath) {
   _dictionary& dict = *dictionaries[category];
   dict.basePath = basePath;
   ctMOReaderRelease(&dict.mo);
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
      ctDynamicArray<uint8_t> fileContents = {};
      CT_RETURN_FAIL(Engine->FileSystem->OpenDataFileByGUID(file, CT_CDATA("LANGUAGES")));
      file.GetBytes(fileContents);
      file.Close();
      ctJSONReader jsonReader = ctJSONReader();
      CT_RETURN_FAIL(
        jsonReader.BuildJsonForPtr((const char*)fileContents.Data(), fileContents.Count()));

      ctJSONReadEntry languagesJson = ctJSONReadEntry();
      jsonReader.GetRootEntry(languagesJson);
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
      dictionaries[category]->mo = {};
      ctStringUtf8 path;
      path.Printf(4096,
                  "text-%s-%s",
                  dictionaries[category]->basePath.CStr(),
                  fullLanguageName.ToLower().CStr());
#if CITRUS_INCLUDE_AUDITION
      // TextHotReload.RegisterPath(path.CStr()); // todo: update to use GUIDs
#endif
      ctFile file;
      CT_RETURN_FAIL(Engine->FileSystem->OpenDataFileByGUID(file, CT_DDATA(path.CStr())));
      ctDynamicArray<uint8_t> fileContents = {};
      file.GetBytes(fileContents);
      file.Close();

      if (!&dictionaries[category]->mo) { ctMOReaderRelease(&dictionaries[category]->mo); }
      ctMOReaderInitialize(&dictionaries[category]->mo, fileContents.Data(), fileContents.Count());
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
   const char* translation = ctMOFindTranslation(&dictionaries[category]->mo, tag);
   if (!translation) { return nativeText; }
   return translation;
}

const char*
ctGetLocalString(ctTranslationCatagory category, const char* tag, const char* nativeText) {
   if (gMainTranslationSystem) {
      return gMainTranslationSystem->GetLocalString(category, tag, nativeText);
   }
   return nativeText;
}