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

#ifdef _WIN32
#include <Windows.h>
#else
#include <sys/stat.h>
#endif

ctFileSystem::ctFileSystem(const ctStringUtf8& appName,
                           const ctStringUtf8& organizationName) {
   _appName = appName;
   _organizationName = organizationName;
}

ctResults ctFileSystem::Startup() {
   ZoneScoped;
   _dataPath = SDL_GetBasePath();
   _prefPath = SDL_GetPrefPath(_organizationName.CStr(), _appName.CStr());

   ctFile assetRedirectFile;
   OpenExeRelativeFile(assetRedirectFile, "assets.redirect");
   char pathSetMode = '~';
   char redirectPath[CT_MAX_FILE_PATH_LENGTH];
   memset(redirectPath, 0, CT_MAX_FILE_PATH_LENGTH);
   assetRedirectFile.ReadRaw(&pathSetMode, 1, 1);
   assetRedirectFile.ReadRaw(redirectPath, 1, CT_MAX_FILE_PATH_LENGTH - 1);
   switch (pathSetMode) {
      case '+':
         _assetPath = _dataPath;
         _assetPath += redirectPath;
         break;
      case '=': _assetPath = redirectPath; break;
      default: _assetPath = _dataPath; break;
   }
   _assetPath += "assets/";
   _assetPath.FilePathLocalize();
   assetRedirectFile.Close();
   return CT_SUCCESS;
}

ctResults ctFileSystem::Shutdown() {
   return CT_SUCCESS;
}

const ctStringUtf8& ctFileSystem::GetPreferencesPath() {
   return _prefPath;
}

const ctStringUtf8& ctFileSystem::GetDataPath() {
   return _dataPath;
}

const ctStringUtf8& ctFileSystem::GetAssetPath() {
   return _assetPath;
}

const void ctFileSystem::LogPaths() {
   ctDebugLog("Preference Path: %s", _prefPath.CStr());
   ctDebugLog("Data Path: %s", _dataPath.CStr());
   ctDebugLog("Asset Path: %s", _assetPath.CStr());
}

const ctResults ctFileSystem::OpenPreferencesFile(ctFile& file,
                                                  const ctStringUtf8& relativePath,
                                                  const ctFileOpenMode mode,
                                                  bool silent) const {
   ctStringUtf8 finalPath = _prefPath;
   finalPath.FilePathAppend(relativePath);
   return file.Open(finalPath, mode, silent);
}

const ctResults ctFileSystem::OpenExeRelativeFile(ctFile& file,
                                                  const ctStringUtf8& relativePath,
                                                  const ctFileOpenMode mode,
                                                  bool silent) const {
   if (mode != CT_FILE_OPEN_READ && mode != CT_FILE_OPEN_READ_TEXT) {
      return CT_FAILURE_INACCESSIBLE;
   }
   ctStringUtf8 finalPath = _dataPath;
   finalPath.FilePathAppend(relativePath);
   return file.Open(finalPath, mode, silent);
}

const ctResults ctFileSystem::OpenAssetFile(ctFile& file,
                                            const ctStringUtf8& relativePath,
                                            const ctFileOpenMode mode,
                                            bool silent) const {
   if (mode != CT_FILE_OPEN_READ && mode != CT_FILE_OPEN_READ_TEXT) {
      return CT_FAILURE_INACCESSIBLE;
   }
   ctStringUtf8 finalPath = _assetPath;
   finalPath.FilePathAppend(relativePath);
   return file.Open(finalPath, mode, silent);
}

const ctResults ctFileSystem::MakePreferencesDirectory(const ctStringUtf8& relativePath) {
   ctStringUtf8 finalPath = _prefPath;
   finalPath += relativePath;
#ifdef _WIN32
   ctDynamicArray<char16_t> wfinalPath;
   finalPath.MakeUTF16Array(wfinalPath);
   LPSECURITY_ATTRIBUTES attr;
   attr = NULL;
   if (CreateDirectory((LPWSTR)wfinalPath.Data(), attr)) { return CT_SUCCESS; }
#else
   if (!mkdir(finalPath.CStr(), (S_IRUSR | S_IWUSR))) { return CT_SUCCESS; }
#endif
   return CT_FAILURE_UNKNOWN;
}