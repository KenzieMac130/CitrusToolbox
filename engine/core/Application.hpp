#pragma once

#include "utilities/Common.h"
#include "FileSystem.hpp"
#include "Logging.hpp"

class ctApplication {
public:
	/* Initialize the engine and all subsystems */
	ctResults Ignite();
	/* Enter the game loop (returns on shutdown) */
	ctResults EnterLoop();
	/* Exit */
	void Exit();
	/* Is Running */
	bool isExitRequested();
	/* Single shot the game loop (returns after frame) */
	ctResults LoopSingleShot(const float deltatime);
	ctResults Shutdown();

	const ctStringUtf8& GetAppName();
	int GetAppVersionMajor();
	int GetAppVersionMinor();
	int GetAppVersionPatch();

	ctFileSystem* FileSystem;
	ctDebugSystem* Debug;
protected:
	/* Event Handling */
	virtual ctResults OnStartup() = 0;
	virtual ctResults OnTick(const float deltatime) = 0;
	virtual ctResults OnShutdown() = 0;
private:
	ctStringUtf8 _appName;
	int _appVersionMajor;
	int _appVersionMinor;
	int _appVersionPatch;

	bool _isRunning = true;
	/* Implimentation */
};