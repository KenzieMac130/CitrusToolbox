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

#include <stdint.h>
#include <stdbool.h>
#include <time.h>

int ctSystemCreateGUID(void* guidPtr);
int ctSystemFilePathLocalize(char* str);
int ctSystemInitialGetLanguage(char* buff, size_t max);
int ctSystemExecuteCommand(const char* commandAlias,
                           int argc,
                           const char* argv[],
                           void (*outputCallback)(const char* output, void* userData),
                           void* userData,
                           const char* workingDirectory);
int ctSystemShowFileToDeveloper(const char* path);
int ctSystemPositionalPrintToString(char* dest, size_t capacity, const char* format, ...);
void* ctSystemMapVirtualFile(const char* path, bool write, size_t reserve, size_t* pSize);
int ctSystemUnmapVirtualFile(void* buff, size_t length);

int ctSystemHostTCPSocket(void* handle, int port, int timeoutMs);
int ctSystemSocketRecv(void* handle, void* buff, int length);
int ctSystemSocketSend(void* handle, void* buff, int length);
void ctSystemCloseSocket(void* handle);

void* ctSystemOpenDir(const char* path);
void ctSystemCloseDir(void* handle);
int ctSystemNextDir(void* handle);

int ctSystemGetDirName(void* handle, char* dest, int max);
int ctSystemIsDirFile(void* handle);
size_t ctSystemGetDirFileSize(void* handle);
time_t ctSystemGetDirDate(void* handle);

int ctSystemFileExists(const char* path);
const char* ctSystemGetGameLayerLibName();