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

/* Defined via codegen */
extern const char* ctGetDataGuidFromHash(size_t hash);

/* Get a guid for the nickname from the asset system with constant string */
#define CT_CDATA(_constname) ctGUID(ctGetDataGuidFromHash(CT_COMPILE_HORNER_HASH(_constname)))
/* Get a guid for the nickname from the asset system with dynamic string  */
#define CT_DDATA(_name) ctGUID(ctGetDataGuidFromHash(ctHornerHash(_name)))

class CT_API ctFileSystem : public ctModuleBase {
public:
   ctFileSystem(const ctStringUtf8& appName, const ctStringUtf8& organizationName);

   ctResults Startup() final;
   ctResults Shutdown() final;

   const void LogPaths();

   const ctResults OpenPreferencesFile(ctFile& file,
                                       const ctStringUtf8& relativePath,
                                       const ctFileOpenMode mode = CT_FILE_OPEN_READ,
                                       bool silent = false) const;
   const ctResults OpenBaseRelativeFile(ctFile& file,
                                        const ctStringUtf8& relativePath,
                                        const ctFileOpenMode mode = CT_FILE_OPEN_READ,
                                        bool silent = false) const;
   const ctResults OpenDataFileByGUID(ctFile& file,
                                      const ctGUID& guid,
                                      const ctFileOpenMode mode = CT_FILE_OPEN_READ,
                                      bool silent = false) const;

   inline const char* GetPreferencesPath() const {
      return _prefPath.CStr();
   }
   inline const char* GetBasePath() const {
      return _basePath.CStr();
   }
   inline const char* GetDataPath() const {
      return _dataPath.CStr();
   }

private:
   ctStringUtf8 _organizationName;
   ctStringUtf8 _appName;
   ctStringUtf8 _prefPath;
   ctStringUtf8 _basePath;
   ctStringUtf8 _dataPath;
};