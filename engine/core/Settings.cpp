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

#include "Settings.hpp"
#include "EngineCore.hpp"

ctResults ctSettingsManager::Startup() {
   ZoneScoped;
   return CT_SUCCESS;
}

ctResults ctSettingsManager::Shutdown() {
   for (auto iter = _sections.GetIterator(); iter; iter++) {
      delete iter.Value();
   }
   return CT_SUCCESS;
}

int ctSettingsManager::FindArgIdx(const char* name) {
   for (int i = 1; i < argc; i++) {
      if (ctCStrEql(argv[i], name)) { return i; }
   }
   return -1;
}

const char* ctSettingsManager::FindArgPairValue(const char* name) {
   int idx = FindArgIdx(name);
   if (idx < 0) { return NULL; }
   if (idx + 1 >= argc) { return NULL; }
   return argv[idx + 1];
}

ctSettingsManager::ctSettingsManager(int _argc, char** _argv) {
   argc = _argc;
   argv = _argv;
}

ctSettingsSection* ctSettingsManager::CreateSection(
  const char* name, int max, ctTranslationCatagory translationCatagory) {
   ZoneScoped;
   const uint32_t hash = XXH32(name, strlen(name), 0);
   ctSettingsSection* section =
     new ctSettingsSection(Engine->FileSystem, this, name, max, translationCatagory);
   return *_sections.Insert(hash, section);
}

ctSettingsSection* ctSettingsManager::GetSection(const char* name) {
   const uint32_t hash = XXH32(name, strlen(name), 0);
   ctSettingsSection** ppSection = _sections.FindPtr(hash);
   if (ppSection) {
      return *ppSection;
   } else {
      return NULL;
   }
}

ctSettingsSection::ctSettingsSection() {
}

ctSettingsSection::ctSettingsSection(ctFileSystem* pFileSystem,
                                     ctSettingsManager* pManager,
                                     const char* name,
                                     int max,
                                     ctTranslationCatagory translationCatagory) {
   _name = name;
   _translationCatagory = translationCatagory;
   _pManager = pManager;
   LoadConfigs(pFileSystem);
}

ctResults ctSettingsSection::_bindvar(_setting_type type,
                                      bool save,
                                      bool load,
                                      const char* name,
                                      const char* help,
                                      void* ptr,
                                      void (*setCallback)(const char* value,
                                                          void* customData),
                                      void* customData,
                                      double min,
                                      double max) {
   ZoneScoped;
   if (!ptr) { return CT_FAILURE_INVALID_PARAMETER; }
   const uint32_t hash = XXH32(name, strlen(name), 0);
   const _setting setting = _setting {
     false, type, save, load, name, help, ptr, setCallback, customData, min, max};
   _settings.Insert(hash, setting);
   return CT_SUCCESS;
}

ctResults ctSettingsSection::BindFloat(float* ptr,
                                       bool save,
                                       bool load,
                                       const char* name,
                                       const char* help,
                                       float min,
                                       float max,
                                       void (*setCallback)(const char* value,
                                                           void* customData),
                                       void* customData) {
   CT_RETURN_FAIL(_bindvar(SETTING_TYPE_FLOAT,
                           save,
                           load,
                           name,
                           help,
                           (void*)ptr,
                           setCallback,
                           customData,
                           (double)min,
                           (double)max));
   if (load && ptr) { GetFallbackFloat(name, *ptr); }
   if (ptr) { ctDebugLog("%s.%s: %f", _name.CStr(), name, *ptr); }
   return CT_SUCCESS;
}

ctResults ctSettingsSection::BindInteger(int32_t* ptr,
                                         bool save,
                                         bool load,
                                         const char* name,
                                         const char* help,
                                         int32_t min,
                                         int32_t max,
                                         void (*setCallback)(const char* value,
                                                             void* customData),
                                         void* customData) {
   CT_RETURN_FAIL(_bindvar(SETTING_TYPE_INTEGER,
                           save,
                           load,
                           name,
                           help,
                           (void*)ptr,
                           setCallback,
                           customData,
                           (double)min,
                           (double)max));
   if (load && ptr) { GetFallbackInteger(name, *ptr); }
   if (ptr) { ctDebugLog("%s.%s: %d", _name.CStr(), name, *ptr); }
   return CT_SUCCESS;
}

ctResults ctSettingsSection::BindString(ctStringUtf8* ptr,
                                        bool save,
                                        bool load,
                                        const char* name,
                                        const char* help,
                                        void (*setCallback)(const char* value,
                                                            void* customData),
                                        void* customData) {
   CT_RETURN_FAIL(_bindvar(
     SETTING_TYPE_STRING, save, load, name, help, (void*)ptr, setCallback, customData));
   if (load && ptr) { GetFallbackString(name, *ptr); }
   if (ptr) { ctDebugLog("%s.%s: %s", _name.CStr(), name, ptr->CStr()); }
   return CT_SUCCESS;
}

ctResults ctSettingsSection::BindFunction(const char* name,
                                          const char* help,
                                          void (*setCallback)(const char* value,
                                                              void* customData),
                                          void* customData) {
   CT_RETURN_FAIL(_bindvar(
     SETTING_TYPE_FUNCTION, false, false, name, help, NULL, setCallback, customData));
   return CT_SUCCESS;
}

ctResults ctSettingsSection::GetFallbackFloat(const char* name, float& out) {
   ctStringUtf8 resolvedVarName;
   resolvedVarName.Printf(128, "-%s.%s", _name.CStr(), name);
   const char* arg = _pManager->FindArgPairValue(resolvedVarName.CStr());
   /* Read from args */
   if (arg) {
      out = (float)atof(arg);
      return CT_SUCCESS;
   }
   /* Read from user */
   {
      ctJSONReadEntry root = ctJSONReadEntry();
      ctJSONReadEntry var = ctJSONReadEntry();
      userJson.GetRootEntry(root);
      root.GetObjectEntry(name, var);
      if (var.isNumber()) {
         var.GetNumber(out);
         return CT_SUCCESS;
      }
   }
   /* Read from json */
   {
      ctJSONReadEntry root = ctJSONReadEntry();
      ctJSONReadEntry var = ctJSONReadEntry();
      defaultJson.GetRootEntry(root);
      root.GetObjectEntry(name, var);
      if (var.isNumber()) {
         var.GetNumber(out);
         return CT_SUCCESS;
      }
   }
   return CT_FAILURE_NOT_FOUND;
}

ctResults ctSettingsSection::GetFallbackInteger(const char* name, int32_t& out) {
   ctStringUtf8 resolvedVarName;
   resolvedVarName.Printf(128, "-%s.%s", _name.CStr(), name);
   const char* arg = _pManager->FindArgPairValue(resolvedVarName.CStr());
   /* Read from args */
   if (arg) {
      out = (int)atoi(arg);
      return CT_SUCCESS;
   }
   /* Read from user */
   {
      ctJSONReadEntry root = ctJSONReadEntry();
      ctJSONReadEntry var = ctJSONReadEntry();
      userJson.GetRootEntry(root);
      root.GetObjectEntry(name, var);
      if (var.isNumber()) {
         var.GetNumber(out);
         return CT_SUCCESS;
      } else if (var.isBool()) {
         bool b = false;
         var.GetBool(b);
         if (b) {
            out = 1;
         } else {
            out = 0;
         }
      }
   }
   /* Read from json */
   {
      ctJSONReadEntry root = ctJSONReadEntry();
      ctJSONReadEntry var = ctJSONReadEntry();
      defaultJson.GetRootEntry(root);
      root.GetObjectEntry(name, var);
      if (var.isNumber()) {
         var.GetNumber(out);
         return CT_SUCCESS;
      } else if (var.isBool()) {
         bool b = false;
         var.GetBool(b);
         if (b) {
            out = 1;
         } else {
            out = 0;
         }
      }
   }
   return CT_FAILURE_NOT_FOUND;
}

ctResults ctSettingsSection::GetFallbackString(const char* name, ctStringUtf8& out) {
   ctStringUtf8 resolvedVarName;
   resolvedVarName.Printf(128, "-%s.%s", _name.CStr(), name);
   const char* arg = _pManager->FindArgPairValue(resolvedVarName.CStr());
   /* Read from args */
   if (arg) {
      out = arg;
      return CT_SUCCESS;
   }
   /* Read from user */
   {
      ctJSONReadEntry root = ctJSONReadEntry();
      ctJSONReadEntry var = ctJSONReadEntry();
      userJson.GetRootEntry(root);
      root.GetObjectEntry(name, var);
      if (var.isString()) {
         var.GetString(out);
         return CT_SUCCESS;
      }
   }
   /* Read from json */
   {
      ctJSONReadEntry root = ctJSONReadEntry();
      ctJSONReadEntry var = ctJSONReadEntry();
      defaultJson.GetRootEntry(root);
      root.GetObjectEntry(name, var);
      if (var.isString()) {
         var.GetString(out);
         return CT_SUCCESS;
      }
   }
   return CT_FAILURE_NOT_FOUND;
}

ctResults
ctSettingsSection::ExecCommand(const char* name, const char* command, bool markChanged) {
   ZoneScoped;
   const uint32_t hash = XXH32(name, strlen(name), 0);
   _setting* pSetting = _settings.FindPtr(hash);
   if (!pSetting) { return CT_FAILURE_DATA_DOES_NOT_EXIST; }
   if (pSetting->setCallback) {
      pSetting->setCallback(command, pSetting->customData);
      return CT_SUCCESS;
   } else {
      ctStringUtf8 cmdStr = command;
      if (pSetting->type == SETTING_TYPE_INTEGER) {
         if (!cmdStr.isInteger()) { return CT_FAILURE_INVALID_PARAMETER; }
         int32_t* pData = (int32_t*)pSetting->dataPtr;
         ctAssert(pData);
         int32_t val = atoi(command);
         if (val > pSetting->maximum) { val = (int32_t)pSetting->maximum; }
         if (val < pSetting->minimum) { val = (int32_t)pSetting->minimum; }
         *pData = val;
         if (markChanged) { pSetting->changed = true; }
         return CT_SUCCESS;
      } else if (pSetting->type == SETTING_TYPE_FLOAT) {
         if (!cmdStr.isNumber()) { return CT_FAILURE_INVALID_PARAMETER; }
         float* pData = (float*)pSetting->dataPtr;
         ctAssert(pData);
         float val = (float)atof(command);
         if (val > pSetting->maximum) { val = (float)pSetting->maximum; }
         if (val < pSetting->minimum) { val = (float)pSetting->minimum; }
         *pData = val;
         if (markChanged) { pSetting->changed = true; }
         return CT_SUCCESS;
      } else if (pSetting->type == SETTING_TYPE_STRING) {
         ctStringUtf8* pData = (ctStringUtf8*)pSetting->dataPtr;
         ctAssert(pData);
         *pData = command;
         if (markChanged) { pSetting->changed = true; }
         return CT_SUCCESS;
      }
      return CT_FAILURE_UNKNOWN;
   }
   return CT_FAILURE_UNKNOWN;
}

ctResults ctSettingsSection::GetValueStr(const char* name, ctStringUtf8& out) {
   ZoneScoped;
   const uint32_t hash = XXH32(name, strlen(name), 0);
   _setting* pSetting = _settings.FindPtr(hash);
   if (!pSetting) { return CT_FAILURE_DATA_DOES_NOT_EXIST; }
   if (pSetting->type == SETTING_TYPE_INTEGER) {
      ctAssert(pSetting->dataPtr);
      out.Printf(32, "%d", (int32_t*)pSetting->dataPtr);
      return CT_SUCCESS;
   } else if (pSetting->type == SETTING_TYPE_FLOAT) {
      ctAssert(pSetting->dataPtr);
      out.Printf(32, "%f", (int32_t*)pSetting->dataPtr);
      return CT_SUCCESS;
   } else if (pSetting->type == SETTING_TYPE_STRING) {
      ctAssert(pSetting->dataPtr);
      const ctStringUtf8* str = (ctStringUtf8*)pSetting->dataPtr;
      out += *(ctStringUtf8*)pSetting->dataPtr;
      return CT_SUCCESS;
   }
   return CT_FAILURE_DATA_DOES_NOT_EXIST;
}

ctResults ctSettingsSection::GetHelp(const char* name, ctStringUtf8& out) {
   ZoneScoped;
   const uint32_t hash = XXH32(name, strlen(name), 0);
   _setting* pSetting = _settings.FindPtr(hash);
   if (!pSetting) { return CT_FAILURE_DATA_DOES_NOT_EXIST; }
   ctAssert(pSetting->help);
   ctStringUtf8 helpSectionName;
   helpSectionName.Printf(256, "CFGHELP:%s:%s", _name.CStr(), name);
   const char* text =
     ctGetLocalString(_translationCatagory, helpSectionName.CStr(), pSetting->help);
   out += text;
   return CT_SUCCESS;
}

ctResults ctSettingsSection::LoadConfigs(ctFileSystem* pFileSystem) {
   ctFile prefFile;
   ctStringUtf8 path;
   path.Printf(256, "%s.json", _name.CStr());
   if (pFileSystem->OpenPreferencesFile(prefFile, path, CT_FILE_OPEN_READ, true) ==
       CT_SUCCESS) {
      /* Load pref data */
      prefFile.GetBytes(userJsonBytes);
      if (userJson.BuildJsonForPtr(userJsonBytes.Data(), userJsonBytes.Count()) !=
          CT_SUCCESS) {
         ctDebugError("Bad %s user file!", _name.CStr());
      }
      prefFile.Close();
   } else {
      userJsonBytes.Clear();
      userJson = {};
   }
   ctFile defFile;
   path = "";
   path.Printf(256, "defaults/%s.json", _name.CStr());
   if (pFileSystem->OpenAssetFileNamed(defFile, path.CStr(), CT_FILE_OPEN_READ, false) ==
       CT_SUCCESS) {
      /* Load default data */
      defFile.GetBytes(defaultJsonBytes);
      if (defaultJson.BuildJsonForPtr(defaultJsonBytes.Data(),
                                      defaultJsonBytes.Count()) != CT_SUCCESS) {
         ctDebugError("Bad %s defaults file!", _name.CStr());
      }
      defFile.Close();
   } else {
      defaultJsonBytes.Clear();
      defaultJson = {};
   }
   return CT_SUCCESS;
}

ctResults ctSettingsSection::SaveChanged(ctFileSystem* pFileSystem) {
   return ctResults();
}
