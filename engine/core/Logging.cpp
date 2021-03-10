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

#include "Logging.hpp"
#include "EngineCore.hpp"

ctDebugSystem::ctDebugSystem(uint32_t flushafter) {
   _flushAfter = flushafter;
   _logLock = ctMutexCreate();
}

ctDebugSystem::~ctDebugSystem() {
   ctMutexDestroy(_logLock);
}

ctResults ctDebugSystem::Startup() {
   Engine->FileSystem->OpenPreferencesFile(
     _logFile, "Log.txt", CT_FILE_OPEN_WRITE);
   return CT_SUCCESS;
}

ctResults ctDebugSystem::Shutdown() {
   ctMutexLock(_logLock);
   _flushMessageQueue();
   _logFile.Close();
   ctMutexUnlock(_logLock);
   return CT_SUCCESS;
}

void ctDebugSystem::Log(const char* format, ...) {
   va_list args;
   va_start(args, format);
   char tmp[CT_MAX_LOG_LENGTH];
   memset(tmp, 0, CT_MAX_LOG_LENGTH);
   vsnprintf(tmp, CT_MAX_LOG_LENGTH-1, format, args);
   ctMutexLock(_logLock);
   printf("[LOG] %s\n", tmp);
   _internalMessage msg;
   strncpy(msg.msg, tmp, CT_MAX_LOG_LENGTH);
   msg.level = 0;
   _addToMessageQueue(msg);
   ctMutexUnlock(_logLock);
   va_end(args);
}

void ctDebugSystem::Warning(const char* format, ...) {
   va_list args;
   va_start(args, format);
   char tmp[CT_MAX_LOG_LENGTH];
   memset(tmp, 0, CT_MAX_LOG_LENGTH);
   vsnprintf(tmp, CT_MAX_LOG_LENGTH-1, format, args);
   ctMutexLock(_logLock);
   printf("[WARNING] %s\n", tmp);
   _internalMessage msg;
   strncpy(msg.msg, tmp, CT_MAX_LOG_LENGTH);
   msg.level = 1;
   _addToMessageQueue(msg);
   ctMutexUnlock(_logLock);
   va_end(args);
}

void ctDebugSystem::Error(const char* format, ...) {
   va_list args;
   va_start(args, format);
   char tmp[CT_MAX_LOG_LENGTH];
   memset(tmp, 0, CT_MAX_LOG_LENGTH);
   vsnprintf(tmp, CT_MAX_LOG_LENGTH-1, format, args);
   ctMutexLock(_logLock);
   printf("[ERROR] %s\n", tmp);
   _internalMessage msg;
   strncpy(msg.msg, tmp, CT_MAX_LOG_LENGTH);
   msg.level = 2;
   _addToMessageQueue(msg);
   ctMutexUnlock(_logLock);
   va_end(args);
}

void ctDebugSystem::_flushMessageQueue() {
   for (int i = 0; i < _messageQueue.Count(); i++) {
      const _internalMessage msg = _messageQueue[i];
      if (msg.level > 2) { continue; }
      const char* levelStr[] = {
        "[LOG] %.*s\n", "[ERROR] %.*s\n", "[WARNING] %.*s\n"};
      _logFile.Printf(levelStr[msg.level], 512, msg.msg);
   }
}

void ctDebugSystem::_addToMessageQueue(const _internalMessage msg) {
   _messageQueue.Append(msg);
   if (_messageQueue.Count() > _flushAfter) {
      _flushMessageQueue();
      _messageQueue.Clear();
   }
}
