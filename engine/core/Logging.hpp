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
#include "FileSystem.hpp"

#include "ModuleBase.hpp"

class CT_API ctDebugSystem : public ctModuleBase {
public:
   ctDebugSystem(uint32_t flushafter, bool shared_log);
   ~ctDebugSystem();

   ctResults Startup() final;
   ctResults Shutdown() final;
   const char* GetModuleName() final;
   virtual void DebugUI(bool useGizmos);

   void Log(const char* format, ...);
   void LogArgs(const char* format, va_list args);
   void Warning(const char* format, ...);
   void WarningArgs(const char* format, va_list args);
   void Error(const char* format, ...);
   void ErrorArgs(const char* format, va_list args);
   void PopupError(const char* format, ...);
   void PopupErrorArgs(const char* format, va_list args);

   void _EmergencExit();

private:
   struct _internalMessage {
      int level;
      char msg[CT_MAX_LOG_LENGTH];
   };

   void _flushMessageQueue();
   void _addToMessageQueue(const _internalMessage msg);
   ctDynamicArray<_internalMessage> _messageQueue;
   ctDynamicArray<_internalMessage> _messageMemory;
   ctFile _logFile;
   int32_t _writeToConsole;
   int32_t _flushAfter;
   ctMutex _logLock;
   bool _isShared;
};