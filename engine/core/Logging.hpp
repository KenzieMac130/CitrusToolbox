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
#include "FileSystem.hpp"

#include "Tracy.hpp"

#include "ModuleBase.hpp"

class ctDebugSystem : public ctModuleBase {
public:
   ctDebugSystem(uint32_t flushafter);
   ~ctDebugSystem();

   ctResults LoadConfig(ctJSONReader::Entry& json) final;
   ctResults SaveConfig(ctJSONWriter& writer) final;
   ctResults Startup(ctEngineCore* pEngine) final;
   ctResults Shutdown() final;

   void Log(const char* format, ...);
   void Warning(const char* format, ...);
   void Error(const char* format, ...);
   /* Draw Line */
private:
   struct _internalMessage {
      int level;
      char msg[512];
   };

   void _flushMessageQueue();
   void _addToMessageQueue(const _internalMessage msg);
   ctDynamicArray<_internalMessage> _messageQueue;
   ctFile _logFile;
   uint32_t _flushAfter;
   ctMutex _logLock;
};