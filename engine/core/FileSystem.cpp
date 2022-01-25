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

#include "FileSystem.hpp"
#include "EngineCore.hpp"
#include "Translation.hpp"

ctFileSystem::ctFileSystem(const ctStringUtf8& appName,
                           const ctStringUtf8& organizationName) {
   _appName = appName;
   _organizationName = organizationName;
}

ctResults ctFileSystem::Startup() {
   ZoneScoped;
   _basePath = SDL_GetBasePath();
   _prefPath = SDL_GetPrefPath(_organizationName.CStr(), _appName.CStr());

   ctFile dataRedirectFile;
   OpenBaseRelativeFile(dataRedirectFile, "data.redirect");
   char pathSetMode = '~';
   char redirectPath[CT_MAX_FILE_PATH_LENGTH];
   memset(redirectPath, 0, CT_MAX_FILE_PATH_LENGTH);
   dataRedirectFile.ReadRaw(&pathSetMode, 1, 1);
   dataRedirectFile.ReadRaw(redirectPath, 1, CT_MAX_FILE_PATH_LENGTH - 1);
   switch (pathSetMode) {
      case '+':
         _dataPath = _basePath;
         _dataPath += redirectPath;
         break;
      case '=': _dataPath = redirectPath; break;
      default: _dataPath = _basePath; break;
   }
   _dataPath.FilePathLocalize();
   dataRedirectFile.Close();
   return CT_SUCCESS;
}

ctResults ctFileSystem::Shutdown() {
   return CT_SUCCESS;
}

const void ctFileSystem::LogPaths() {
   ctDebugLog("Preference Path: %s", _prefPath.CStr());
   ctDebugLog("Executable Path: %s", _basePath.CStr());
   ctDebugLog("Data Path: %s", _dataPath.CStr());
}

const ctResults ctFileSystem::OpenPreferencesFile(ctFile& file,
                                                  const ctStringUtf8& relativePath,
                                                  const ctFileOpenMode mode,
                                                  bool silent) const {
   ctStringUtf8 finalPath = _prefPath;
   finalPath.FilePathAppend(relativePath);
   return file.Open(finalPath, mode, silent);
}

const ctResults ctFileSystem::OpenBaseRelativeFile(ctFile& file,
                                                   const ctStringUtf8& relativePath,
                                                   const ctFileOpenMode mode,
                                                   bool silent) const {
   if (mode != CT_FILE_OPEN_READ && mode != CT_FILE_OPEN_READ_TEXT) {
      return CT_FAILURE_INACCESSIBLE;
   }
   ctStringUtf8 finalPath = _basePath;
   finalPath.FilePathAppend(relativePath);
   return file.Open(finalPath, mode, silent);
}

const ctResults ctFileSystem::OpenDataFileByGUID(ctFile& file,
                                                 const ctGUID& guid,
                                                 const ctFileOpenMode mode,
                                                 bool silent) const {
   if (mode != CT_FILE_OPEN_READ && mode != CT_FILE_OPEN_READ_TEXT) {
      return CT_FAILURE_INACCESSIBLE;
   }

   ctStringUtf8 finalPath = _dataPath;
   char hexData[33];
   memset(hexData, 0, 33);
   guid.ToHex(hexData);
   const char* guidStr = ctGetLocalString(CT_TRANSLATION_CATAGORY_DATA, hexData, hexData);
   finalPath.FilePathAppend(guidStr);
   return file.Open(finalPath, mode, silent);
}