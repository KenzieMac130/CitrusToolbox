#pragma once

#include "utilities/Common.h"
#include "FileSystem.hpp"

#include "Tracy.hpp"

class ctDebugSystem
{
public:
	ctDebugSystem(ctFileSystem* pFSystem, uint32_t flushafter);
	~ctDebugSystem();

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