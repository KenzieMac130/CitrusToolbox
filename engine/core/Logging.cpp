#include "Logging.hpp"

ctDebugSystem::ctDebugSystem(ctFileSystem* pFSystem, uint32_t flushafter) {
   _logLock = ctMutexCreate();
   _flushAfter = flushafter;
   pFSystem->OpenPreferencesFile(_logFile, "Log.txt", CT_FILE_OPEN_WRITE);
}

ctDebugSystem::~ctDebugSystem() {
   ctMutexLock(_logLock);
   _flushMessageQueue();
   _logFile.Close();
   ctMutexUnlock(_logLock);
   ctMutexDestroy(_logLock);
}

void ctDebugSystem::Log(const char* format, ...) {
   va_list args;
   va_start(args, format);
   char tmp[512];
   memset(tmp, 0, 512);
   vsnprintf(tmp, 511, format, args);
   ctMutexLock(_logLock);
   printf("[LOG] %s\n", tmp);
   _internalMessage msg;
   strncpy(msg.msg, tmp, 512);
   msg.level = 0;
   _addToMessageQueue(msg);
   ctMutexUnlock(_logLock);
   va_end(args);
}

void ctDebugSystem::Warning(const char* format, ...) {
   va_list args;
   va_start(args, format);
   char tmp[512];
   memset(tmp, 0, 512);
   vsnprintf(tmp, 511, format, args);
   ctMutexLock(_logLock);
   printf("[WARNING] %s\n", tmp);
   _internalMessage msg;
   strncpy(msg.msg, tmp, 512);
   msg.level = 1;
   _addToMessageQueue(msg);
   ctMutexUnlock(_logLock);
   va_end(args);
}

void ctDebugSystem::Error(const char* format, ...) {
   va_list args;
   va_start(args, format);
   char tmp[512];
   memset(tmp, 0, 512);
   vsnprintf(tmp, 511, format, args);
   ctMutexLock(_logLock);
   printf("[ERROR] %s\n", tmp);
   _internalMessage msg;
   strncpy(msg.msg, tmp, 512);
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
