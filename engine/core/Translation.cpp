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
#include "FileSystem.hpp"
#include "Settings.hpp"

#ifdef _WIN32
#include <Windows.h>
#endif
#include <locale.h>

ctTranslation* mainTranslationSystem;

ctTranslation::ctTranslation(bool shared) {
   if (shared) { mainTranslationSystem = this; }
   for (int i = 0; i < CT_TRANSLATION_CATAGORY_COUNT; i++) {
      dictionaries.Append(new _dictionary());
   }
}

void _setLanguageCb(const char* val, void* data) {
   ctTranslation* pTranslation = (ctTranslation*)data;
   pTranslation->LoadLanguage(val);
}

ctResults ctTranslation::Startup() {
   ZoneScoped;
#if CITRUS_INCLUDE_AUDITION
   Engine->HotReload->RegisterAssetCategory(&TextHotReload);
#endif
#if defined(_WIN32)
   wchar_t data[LOCALE_NAME_MAX_LENGTH];
   GetLocaleInfoEx(LOCALE_NAME_USER_DEFAULT, LOCALE_SNAME, data, LOCALE_NAME_MAX_LENGTH);
   isoLanguage = ctStringUtf8(data);
#elif defined(__linux__)
   /* Try to extract a similar string off "setlocale" (tested only on some Debian distros)
    */
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
   isoLanguage = scratch;
   if (isoLanguage == "C") { isoLanguage = "DEFAULT"; }
#else
   isoLanguage = "DEFAULT";
#endif
   setlocale(LC_ALL, "C"); /* Unify C Locale */
   fullLanguageName = "?";
   ctDebugLog("OS Reported Language: %s", isoLanguage.CStr());

   ctSettingsSection* settings = Engine->Settings->CreateSection("Translation", 1);
   settings->BindString(&isoLanguage,
                        true,
                        true,
                        "Language",
                        "Code for the language to use in RFC 4646 format.");

   SetDictionary(CT_TRANSLATION_CATAGORY_CORE, "text");
   LoadLanguage(isoLanguage.CStr());
   return CT_SUCCESS;
}

ctResults ctTranslation::Shutdown() {
   for (size_t i = 0; i < dictionaries.Count(); i++) {
      if (dictionaries[i]) { delete dictionaries[i]; }
   }
   return CT_SUCCESS;
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

ctResults ctTranslation::SetDictionary(ctTranslationCatagory category,
                                       const char* basePath) {
   _dictionary& dict = *dictionaries[category];
   dict.basePath = basePath;
   dict.strings.Clear();
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
      CT_RETURN_FAIL(Engine->FileSystem->OpenAssetFile(file, "text/languages.json"));
      file.GetBytes(fileContents);
      file.Close();
      ctJSONReader jsonReader = ctJSONReader();
      CT_RETURN_FAIL(jsonReader.BuildJsonForPtr((const char*)fileContents.Data(),
                                                fileContents.Count()));

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
      ctStringUtf8 path;
      path.Printf(4096,
                  "%s/%s.json",
                  dictionaries[category]->basePath.CStr(),
                  fullLanguageName.ToLower().CStr());
#if CITRUS_INCLUDE_AUDITION
      TextHotReload.RegisterPath(path.CStr());
#endif
      ctFile file;
      ctDynamicArray<uint8_t> fileContents = {};
      CT_RETURN_FAIL(Engine->FileSystem->OpenAssetFile(file, path.CStr()));
      file.GetBytes(fileContents);
      file.Close();
      ctJSONReader jsonReader = ctJSONReader();
      CT_RETURN_FAIL(jsonReader.BuildJsonForPtr((const char*)fileContents.Data(),
                                                fileContents.Count()));

      dictionaries[category]->bloom.Reset();
      dictionaries[category]->strings.Clear();

      ctJSONReadEntry textJson = ctJSONReadEntry();
      jsonReader.GetRootEntry(textJson);
      int entryCount = textJson.GetObjectEntryCount();
      dictionaries[category]->strings.Reserve(entryCount);
      for (int i = 0; i < entryCount; i++) {
         ctJSONReadEntry entry = ctJSONReadEntry();
         ctStringUtf8 name = ctStringUtf8();
         ctStringUtf8 content = ctStringUtf8();
         textJson.GetObjectEntry(i, entry, &name);
         entry.GetString(content);
         if (content.isEmpty()) { continue; }
         content.ProcessEscapeCodes();
         const uint64_t hash = name.xxHash64();
         dictionaries[category]->bloom.Insert(hash);
         dictionaries[category]->strings.Insert(hash, content);
      }
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
   uint64_t hash = ctXXHash64(tag);
   if (!dictionaries[category]->bloom.MightExist(hash)) { return nativeText; }
   const ctStringUtf8* localText = dictionaries[category]->strings.FindPtr(hash);
   if (localText) { return localText->CStr(); }
   return nativeText;
}

const char* ctGetLocalString(ctTranslationCatagory category,
                             const char* tag,
                             const char* nativeText) {
   if (mainTranslationSystem) {
      return mainTranslationSystem->GetLocalString(category, tag, nativeText);
   }
   return nativeText;
}
