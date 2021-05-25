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
#include "FileSystem.hpp"
#include "Translation.hpp"

#define CT_SETTINGS_BOUNDS_BOOL 0,1
#define CT_SETTINGS_BOUNDS_UINT 0,UINT32_MAX

class CT_API ctSettingsSection {
public:
   ctSettingsSection();
   ctSettingsSection(int max, ctTranslationCatagory translationCatagory);
   ctResults BindInteger(int32_t* ptr,
                         bool save,
                         bool load,
                         const char* name,
                         const char* help,
                         int32_t min = INT32_MIN,
                         int32_t max = INT32_MAX,
                         void (*setCallback)(const char* value,
                                             void* customData) = NULL,
                         void* customData = NULL);
   ctResults BindFloat(float* ptr,
                       bool save,
                       bool load,
                       const char* name,
                       const char* help,
                       float min = -FLT_MAX,
                       float max = FLT_MAX,
                       void (*setCallback)(const char* value,
                                           void* customData) = NULL,
                       void* customData = NULL);
   ctResults BindString(ctStringUtf8* ptr,
                        bool save,
                        bool load,
                        const char* name,
                        const char* help,
                        void (*setCallback)(const char* value,
                                            void* customData) = NULL,
                        void* customData = NULL);
   ctResults BindFunction(const char* name,
                          const char* help,
                          void (*setCallback)(const char* value,
                                              void* customData) = NULL,
                          void* customData = NULL);

   ctResults ExecCommand(const char* name, const char* command);
   ctResults GetValueStr(const char* name, ctStringUtf8& out);
   ctResults GetHelp(const char* name, ctStringUtf8& out);

private:
   enum _setting_type {
      SETTING_TYPE_FLOAT,
      SETTING_TYPE_INTEGER,
      SETTING_TYPE_STRING,
      SETTING_TYPE_FUNCTION,
   };

   ctResults _bindvar(_setting_type type,
                      bool save,
                      bool load,
                      const char* name,
                      const char* help,
                      void* ptr,
                      void (*setCallback)(const char* value, void* customData),
                      void* customData,
                      double min = -DBL_MAX,
                      double max = DBL_MAX);

   struct _setting {
      _setting_type type;
      bool save;
      bool load;
      const char* name;
      const char* help;
      void* dataPtr;
      void (*setCallback)(const char* value, void* customData);
      void* customData;
      double minimum;
      double maximum;
   };
   ctTranslationCatagory _translationCatagory;
   ctHashTable<_setting, uint32_t> _settings;
};

class CT_API ctSettings : public ctModuleBase {
public:
   ctSettingsSection* CreateSection(
     const char* name,
     int max,
     ctTranslationCatagory translationCatagory = CT_TRANSLATION_CATAGORY_CORE);
   ctSettingsSection* GetSection(const char* name);

   ctResults Startup() final;
   ctResults Shutdown() final;

private:
   ctHashTable<ctSettingsSection, uint32_t> _sections;
};