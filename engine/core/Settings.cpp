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

ctResults ctSettings::Startup() {
   _sections = ctHashTable<ctSettingsSection>(CT_MAX_SETTINGS_SECTIONS);
   return CT_SUCCESS;
}

ctResults ctSettings::Shutdown() {
   return CT_SUCCESS;
}

ctSettingsSection* ctSettings::CreateSection(const char* name, int max) {
   const uint32_t hash = XXH32(name, strlen(name), 0);
   return _sections.Insert(hash, ctSettingsSection(max));
}

ctSettingsSection* ctSettings::GetSection(const char* name) {
   const uint32_t hash = XXH32(name, strlen(name), 0);
   return _sections.FindPtr(hash);
}

ctSettingsSection::ctSettingsSection() {
   _settings = ctHashTable<_setting>();
}

ctSettingsSection::ctSettingsSection(int max) {
   _settings = ctHashTable<_setting>(max);
}

ctResults ctSettingsSection::_bindvar(_setting_type type,
                                      bool save,
                                      bool load,
                                      const char* name,
                                      const char* help,
                                      void* ptr,
                                      void (*setCallback)(const char* value,
                                                          void* customData),
                                      void* customData) {
   if (!ptr) { return CT_FAILURE_INVALID_PARAMETER; }
   const uint32_t hash = XXH32(name, strlen(name), 0);
   const _setting setting =
     _setting {type, save, load, name, help, ptr, setCallback, customData};
   if (_settings.Insert(hash, setting) != NULL) {
      return CT_FAILURE_DUPLICATE_ENTRY;
   };
   return CT_SUCCESS;
}

ctResults ctSettingsSection::BindInteger(int32_t* ptr,
                                         bool save,
                                         bool load,
                                         const char* name,
                                         const char* help,
                                         void (*setCallback)(const char* value,
                                                             void* customData),
                                         void* customData) {
   return _bindvar(SETTING_TYPE_INTEGER,
                   save,
                   load,
                   name,
                   help,
                   (void*)ptr,
                   setCallback,
                   customData);
}

ctResults ctSettingsSection::BindFloat(float* ptr,
                                       bool save,
                                       bool load,
                                       const char* name,
                                       const char* help,
                                       void (*setCallback)(const char* value,
                                                           void* customData),
                                       void* customData) {
   return _bindvar(SETTING_TYPE_FLOAT,
                   save,
                   load,
                   name,
                   help,
                   (void*)ptr,
                   setCallback,
                   customData);
}

ctResults ctSettingsSection::BindString(ctStringUtf8* ptr,
                                        bool save,
                                        bool load,
                                        const char* name,
                                        const char* help,
                                        void (*setCallback)(const char* value,
                                                            void* customData),
                                        void* customData) {
   return _bindvar(SETTING_TYPE_STRING,
                   save,
                   load,
                   name,
                   help,
                   (void*)ptr,
                   setCallback,
                   customData);
}

ctResults ctSettingsSection::BindFunction(const char* name,
                                          const char* help,
                                          void (*setCallback)(const char* value,
                                                              void* customData),
                                          void* customData) {
   return _bindvar(SETTING_TYPE_STRING,
                   false,
                   false,
                   name,
                   help,
                   NULL,
                   setCallback,
                   customData);
}

ctResults ctSettingsSection::ExecCommand(const char* name,
                                         const char* command) {
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
         *pData = atoi(command);
         return CT_SUCCESS;
      } else if (pSetting->type == SETTING_TYPE_FLOAT) {
         if (!cmdStr.isNumber()) { return CT_FAILURE_INVALID_PARAMETER; }
         float* pData = (float*)pSetting->dataPtr;
         ctAssert(pData);
         *pData = (float)atof(command);
         return CT_SUCCESS;
      } else if (pSetting->type == SETTING_TYPE_STRING) {
         ctStringUtf8* pData = (ctStringUtf8*)pSetting->dataPtr;
         ctAssert(pData);
         *pData = command;
         return CT_SUCCESS;
      }
   }
   return CT_FAILURE_UNKNOWN;
}

ctResults ctSettingsSection::GetValue(const char* name, ctStringUtf8& out) {
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
   const uint32_t hash = XXH32(name, strlen(name), 0);
   _setting* pSetting = _settings.FindPtr(hash);
   if (!pSetting) { return CT_FAILURE_DATA_DOES_NOT_EXIST; }
   ctAssert(pSetting->help);
   out += pSetting->help;
   return CT_SUCCESS;
}