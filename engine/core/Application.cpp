#include "Application.hpp"

ctResults ctApplication::Ignite() {
   /*Setup modules*/
   FileSystem = new ctFileSystem("appName", "CitrusToolbox");
   Debug = new ctDebugSystem(FileSystem, 32);
   /*Run User Code*/
   OnStartup();
   return CT_SUCCESS;
}

ctResults ctApplication::EnterLoop() {
   _isRunning = true;
   while (_isRunning) {
      LoopSingleShot(1.0f / 60.0f);
   }
   Shutdown();
   return CT_SUCCESS;
}

void ctApplication::Exit() {
   _isRunning = false;
}

bool ctApplication::isExitRequested() {
   return !_isRunning;
}

ctResults ctApplication::LoopSingleShot(const float deltatime) {
   /*Update modules*/
   OnTick(deltatime);
   return CT_SUCCESS;
}

ctResults ctApplication::Shutdown() {
   OnShutdown();
   /*Shutdown modules*/
   delete Debug;
   delete FileSystem;
   return CT_SUCCESS;
}

const ctStringUtf8& ctApplication::GetAppName() {
   return _appName;
}

int ctApplication::GetAppVersionMajor() {
   return _appVersionMajor;
}

int ctApplication::GetAppVersionMinor() {
   return _appVersionMinor;
}

int ctApplication::GetAppVersionPatch() {
   return _appVersionPatch;
}
