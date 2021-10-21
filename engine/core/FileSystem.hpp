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

class CT_API ctFileSystem : public ctModuleBase {
public:
   ctFileSystem(const ctStringUtf8& appName, const ctStringUtf8& organizationName);

   ctResults Startup() final;
   ctResults Shutdown() final;

   const ctStringUtf8& GetPreferencesPath();
   const ctStringUtf8& GetDataPath();
   const ctStringUtf8& GetAssetPath();

   ctResults BuildAssetManifest();
   const void LogPaths();

   const ctResults OpenPreferencesFile(ctFile& file,
                                       const ctStringUtf8& relativePath,
                                       const ctFileOpenMode mode = CT_FILE_OPEN_READ,
                                       bool silent = false) const;
   const ctResults OpenExeRelativeFile(ctFile& file,
                                       const ctStringUtf8& relativePath,
                                       const ctFileOpenMode mode = CT_FILE_OPEN_READ,
                                       bool silent = false) const;
   const ctResults OpenAssetFileNamed(ctFile& file,
                                      const char* name,
                                      const ctFileOpenMode mode = CT_FILE_OPEN_READ,
                                      bool silent = false) const;
   const ctResults OpenAssetFileGUID(ctFile& file,
                                     const ctGUID& guid,
                                     const ctFileOpenMode mode = CT_FILE_OPEN_READ,
                                     bool silent = false) const;

private:
   ctStringUtf8 _organizationName;
   ctStringUtf8 _appName;
   ctStringUtf8 _prefPath;
   ctStringUtf8 _dataPath;
   ctStringUtf8 _assetPath;

   struct AssetInfo {
      ctGUID guid;
      char relativePath[CT_MAX_FILE_PATH_LENGTH];
   };
   ctHashTable<AssetInfo, uint64_t> assetsByGuidHash;
};