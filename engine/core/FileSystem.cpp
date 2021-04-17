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

ctFile::ctFile() {
   _fSize = -1;
   _fp = NULL;
   _mode = CT_FILE_OPEN_READ;
}

void ctFile::FromCStream(FILE* fp) {
   ZoneScoped;
   Close();
   _fp = fp;
}

ctResults ctFile::Open(const ctStringUtf8& filePath, const ctFileOpenMode mode) {
   ZoneScoped;
   if (mode >= 3 || mode < 0) { return CT_FAILURE_INVALID_PARAMETER; }
   const char* modestr[] = {"rb", "wb", "r", "w"};
   _fp = fopen(filePath.CStr(), modestr[mode]);
   _mode = mode;
   if (_fp) {
      return CT_SUCCESS;
   } else {
      return CT_FAILURE_FILE_INACCESSIBLE;
   }
}

void ctFile::Close() {
   ZoneScoped;
   if (_fp) { fclose(_fp); }
   _fp = NULL;
}

int64_t ctFile::GetFileSize() {
   ZoneScoped;
   if (!_fp) { return 0; }
   if (_fSize == -1) {
      Seek(0, CT_FILE_SEEK_END);
      _fSize = Tell();
   }
   return _fSize;
}

int64_t ctFile::Tell() {
   ZoneScoped;
   if (!_fp) { return 0; }
   return ftell(_fp);
}

ctResults ctFile::Seek(const int64_t offset, const ctFileSeekMode mode) {
   ZoneScoped;
   if (!_fp) { return CT_FAILURE_FILE_INACCESSIBLE; }
   return fseek(_fp, (long)offset, mode) == 0 ? CT_SUCCESS : CT_FAILURE_FILE_INACCESSIBLE;
}

size_t ctFile::ReadRaw(void* pDest, const size_t size, const size_t count) {
   ZoneScoped;
   if (!_fp) { return 0; }
   return fread(pDest, size, count, _fp);
}

size_t ctFile::ReadString(ctStringUtf8& pDest, const size_t count) {
   ZoneScoped;
   // if (!_fp) { return 0; }
   // return fread(pDest, size, count, _fp);
   return 0;
}

size_t ctFile::WriteRaw(const void* pData, size_t size, const size_t count) {
   ZoneScoped;
   if (!_fp) { return 0; }
   return fwrite(pData, size, count, _fp);
}

int64_t ctFile::Printf(const char* format, ...) {
   ZoneScoped;
   va_list args;
   va_start(args, format);
   const int64_t result = VPrintf(format, args);
   va_end(args);
   return result;
}

int64_t ctFile::VPrintf(const char* format, va_list va) {
   ZoneScoped;
   if (!_fp) { return 0; }
   return vfprintf(_fp, format, va);
}

FILE* ctFile::CFile() const {
   return _fp;
}

bool ctFile::isOpen() const {
   return _fp;
}

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
   return ctResults();
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
                                                  const ctFileOpenMode mode) {
   ctStringUtf8 finalPath = _prefPath;
   finalPath += relativePath;
   return file.Open(finalPath, mode);
}

const ctResults ctFileSystem::OpenExeRelativeFile(ctFile& file,
                                                  const ctStringUtf8& relativePath) {
   ctStringUtf8 finalPath = _dataPath;
   finalPath += relativePath;
   return file.Open(finalPath, CT_FILE_OPEN_READ);
}

const ctResults ctFileSystem::OpenAssetFile(ctFile& file,
                                            const ctStringUtf8& relativePath) {
   ctStringUtf8 finalPath = _assetPath;
   finalPath += relativePath;
   return file.Open(finalPath, CT_FILE_OPEN_READ);
}
