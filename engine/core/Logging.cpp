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
#include "core/EngineCore.hpp"

ctDynamicArray<ctDebugSystem*> danglingDebugSystems;
bool savedExit = false;
void saveDanglingDebugSystems(void) {
   for (int i = 0; i < danglingDebugSystems.Count(); i++) {
      danglingDebugSystems[i]->_EmergencExit();
   }
}

ctDebugSystem* mainDebugSystem = NULL;
void shared_callback(int level, const char* format, va_list args) {
   switch (level) {
      case 0: mainDebugSystem->LogArgs(format, args); break;
      case 1: mainDebugSystem->WarningArgs(format, args); break;
      case 2: mainDebugSystem->ErrorArgs(format, args); break;
      case 3: mainDebugSystem->PopupErrorArgs(format, args); break;
      default: break;
   }
}

ctDebugSystem::ctDebugSystem(uint32_t flushafter, bool shared_log) {
   _flushAfter = flushafter;
   _logLock = ctMutexCreate();
   if (shared_log) {
      mainDebugSystem = this;
      _ctDebugLogSetCallback(shared_callback);
   }
}

ctDebugSystem::~ctDebugSystem() {
   ctMutexDestroy(_logLock);
}

ctResults ctDebugSystem::Startup() {
   ZoneScoped;
   ctSettingsSection* settings =
     Engine->Settings->CreateSection("DebugSystem", 1);
   settings->BindInteger(
     &_flushAfter,
     false,
     true,
     "FlushAfter",
     "Write the debug log contents after a certain amount of logs.");

   if (Engine->FileSystem->isStarted()) {
      Engine->FileSystem->OpenPreferencesFile(
        _logFile, "Log.txt", CT_FILE_OPEN_WRITE);
   }
   if (!_logFile.isOpen()) {
      Error("Debug System: Log.txt CANNOT BE OPENED!");
   } else {
      Log("Debug System: Online");
   }
   danglingDebugSystems.Append(this);
   if (!savedExit) { atexit(saveDanglingDebugSystems); }
   return CT_SUCCESS;
}

ctResults ctDebugSystem::Shutdown() {
   ctMutexLock(_logLock);
   _flushMessageQueue();
   _logFile.Close();
   ctMutexUnlock(_logLock);
   danglingDebugSystems.Remove(this);
   return CT_SUCCESS;
}

void ctDebugSystem::_EmergencExit() {
   Warning("Emergency Exit Triggered!");
   ctMutexLock(_logLock);
   _flushMessageQueue();
   _logFile.Close();
   ctMutexUnlock(_logLock);
}

void ctDebugSystem::Log(const char* format, ...) {
   va_list args;
   va_start(args, format);
   LogArgs(format, args);
   va_end(args);
}

void ctDebugSystem::LogArgs(const char* format, va_list args) {
   ZoneScoped;
   char tmp[CT_MAX_LOG_LENGTH];
   memset(tmp, 0, CT_MAX_LOG_LENGTH);
   vsnprintf(tmp, CT_MAX_LOG_LENGTH - 1, format, args);
   ctMutexLock(_logLock);
   fprintf(stdout, "[LOG] %s\n", tmp);
   TracyMessage(tmp, strlen(tmp));
   _internalMessage msg;
   strncpy(msg.msg, tmp, CT_MAX_LOG_LENGTH);
   msg.level = 0;
   _addToMessageQueue(msg);
   ctMutexUnlock(_logLock);
}

void ctDebugSystem::Warning(const char* format, ...) {
   va_list args;
   va_start(args, format);
   WarningArgs(format, args);
   va_end(args);
}

void ctDebugSystem::WarningArgs(const char* format, va_list args) {
   ZoneScoped;
   char tmp[CT_MAX_LOG_LENGTH];
   memset(tmp, 0, CT_MAX_LOG_LENGTH);
   vsnprintf(tmp, CT_MAX_LOG_LENGTH - 1, format, args);
   ctMutexLock(_logLock);
   fprintf(stderr, "[WARNING] %s\n", tmp);
   TracyMessageC(tmp, strlen(tmp), 0xE5A91A);
   _internalMessage msg;
   strncpy(msg.msg, tmp, CT_MAX_LOG_LENGTH);
   msg.level = 1;
   _addToMessageQueue(msg);
   ctMutexUnlock(_logLock);
}

void ctDebugSystem::Error(const char* format, ...) {
   va_list args;
   va_start(args, format);
   ErrorArgs(format, args);
   va_end(args);
}

void ctDebugSystem::ErrorArgs(const char* format, va_list args) {
   ZoneScoped;
   char tmp[CT_MAX_LOG_LENGTH];
   memset(tmp, 0, CT_MAX_LOG_LENGTH);
   vsnprintf(tmp, CT_MAX_LOG_LENGTH - 1, format, args);
   ctMutexLock(_logLock);
   fprintf(stderr, "[ERROR] %s\n", tmp);
   TracyMessageC(tmp, strlen(tmp), 0xE51A1A);
   _internalMessage msg;
   strncpy(msg.msg, tmp, CT_MAX_LOG_LENGTH);
   msg.level = 2;
   _addToMessageQueue(msg);
   ctMutexUnlock(_logLock);
}

void ctDebugSystem::PopupError(const char* format, ...) {
   va_list args;
   va_start(args, format);
   PopupErrorArgs(format, args);
   va_end(args);
}

void ctDebugSystem::PopupErrorArgs(const char* format, va_list args) {
   ZoneScoped;
   ErrorArgs(format, args);
   if (!Engine) { return; }
   if (!Engine->WindowManager) { return; }
   if (!Engine->WindowManager->isStarted()) { return; };
   ctStringUtf8 msg = ctStringUtf8();
   msg.VPrintf(64, format, args);
   Engine->WindowManager->ShowErrorMessage("Fatal Error", msg.CStr());
}

void ctDebugSystem::_flushMessageQueue() {
   ZoneScoped;
   if (!_logFile.isOpen()) { return; }
   for (int i = 0; i < _messageQueue.Count(); i++) {
      const _internalMessage msg = _messageQueue[i];
      if (msg.level > 2) { continue; }
      const char* levelStr[] = {
        "[LOG] %.*s\n",
        "[WARNING] %.*s\n",
        "[ERROR] %.*s\n",
      };
      _logFile.Printf(levelStr[msg.level], 512, msg.msg);
   }
   fflush(_logFile.CFile());
}

void ctDebugSystem::_addToMessageQueue(const _internalMessage msg) {
   _messageQueue.Append(msg);
   if (_messageQueue.Count() >= _flushAfter) {
      _flushMessageQueue();
      _messageQueue.Clear();
   }
}
