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

#pragma once

#include "utilities/Common.h"
#include "ModuleBase.hpp"
#include "FileSystem.hpp"
#include "Translation.hpp"

#define CT_SETTINGS_BOUNDS_BOOL 0, 1
#define CT_SETTINGS_BOUNDS_UINT 0, UINT32_MAX

class CT_API ctSettingsSection {
public:
   friend class ctSettingsManager;
   ctSettingsSection();
   ctSettingsSection(ctFileSystem* pFileSystem,
                     ctSettingsManager* pManager,
                     const char* name,
                     int max,
                     ctTranslationCatagory translationCatagory);
   ctResults BindInteger(int32_t* ptr,
                         bool save,
                         bool load,
                         const char* name,
                         const char* help,
                         int64_t min = INT32_MIN,
                         int64_t max = INT32_MAX,
                         void (*setCallback)(const char* value, void* customData) = NULL,
                         void* customData = NULL);
   ctResults BindFloat(float* ptr,
                       bool save,
                       bool load,
                       const char* name,
                       const char* help,
                       float min = -FLT_MAX,
                       float max = FLT_MAX,
                       void (*setCallback)(const char* value, void* customData) = NULL,
                       void* customData = NULL);
   ctResults BindString(ctStringUtf8* ptr,
                        bool save,
                        bool load,
                        const char* name,
                        const char* help,
                        void (*setCallback)(const char* value, void* customData) = NULL,
                        void* customData = NULL);
   ctResults BindFunction(const char* name,
                          const char* help,
                          void (*setCallback)(const char* value, void* customData) = NULL,
                          void* customData = NULL);

   ctResults GetFallbackInteger(const char* name, int32_t& out);
   ctResults GetFallbackFloat(const char* name, float& out);
   ctResults GetFallbackString(const char* name, ctStringUtf8& out);

   ctResults ExecCommand(const char* name, const char* command, bool markChanged = true);
   ctResults GetValueStr(const char* name, ctStringUtf8& out);
   ctResults GetHelp(const char* name, ctStringUtf8& out);

   ctResults LoadConfigs(ctFileSystem* pFileSystem);
   ctResults SaveChanged(ctFileSystem* pFileSystem);

protected:
   enum SettingType {
      SETTING_TYPE_FLOAT,
      SETTING_TYPE_INTEGER,
      SETTING_TYPE_STRING,
      SETTING_TYPE_FUNCTION,
   };

   ctResults BindVar(SettingType type,
                      bool save,
                      bool load,
                      const char* name,
                      const char* help,
                      void* ptr,
                      void (*setCallback)(const char* value, void* customData),
                      void* customData,
                      double min = -DBL_MAX,
                      double max = DBL_MAX);

   struct Setting {
      bool changed;
      SettingType type;
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
   ctStringUtf8 name;
   ctTranslationCatagory translationCatagory;
   ctHashTable<Setting, uint32_t> settings;
   ctSettingsManager* pManager;

   ctDynamicArray<char> defaultJsonBytes;
   ctDynamicArray<char> userJsonBytes;
   ctJSONReader defaultJson;
   ctJSONReader userJson;
};

class CT_API ctSettingsManager : public ctModuleBase {
public:
   ctSettingsManager(int argc, char** argv);
   ctSettingsSection* CreateSection(
     const char* name,
     int max,
     ctTranslationCatagory translationCatagory = CT_TRANSLATION_CATAGORY_CORE);
   ctSettingsSection* GetOrCreateSection(
     const char* name,
     int max,
     ctTranslationCatagory translationCatagory = CT_TRANSLATION_CATAGORY_CORE);
   ctSettingsSection* GetSection(const char* name);

   ctResults Startup() final;
   ctResults Shutdown() final;
   const char* GetModuleName() final;
   virtual void DebugUI(bool useGizmos);

   int FindArgIdx(const char* name);
   const char* FindArgPairValue(const char* name);

   int argc;
   char** argv;

private:
   ctHashTable<ctSettingsSection*, uint32_t> _sections;
};