/*
   Copyright 2023 MacKenzie Strand

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

#include "LiveSync.hpp"

#define CT_WANT_SOCKET_LIBRARY
#include "system/System.h"

void ctAuditionLiveSync::StartServer() {
   StopServer(); /* cleanup existing */
   ctThreadWaitForExit(serverThread);

   ctMutexLock(lock);
   locked.port = stagedPort;
   locked.shouldRun = true;
   ctThreadCreate(ServerMain, this, "Live Sync");
   ctMutexUnlock(lock);
}

void ctAuditionLiveSync::StopServer() {
   ctMutexLock(lock);
   locked.shouldRun = false;
   ctMutexUnlock(lock);
}

bool ctAuditionLiveSync::isRunning() {
   ctMutexLock(lock);
   bool result = locked.shouldRun;
   ctMutexUnlock(lock);
   return result;
}

int ctAuditionLiveSync::ServerMain(void* pModule) {
   return ((ctAuditionLiveSync*)pModule)->ServerLoop();
}

void ctAuditionLiveSync::DebugUI(bool useGizmos)
{
}

int ctAuditionLiveSync::ServerLoop() {
   ctDebugLog("Audition Live Socket Startup");

   ctMutexLock(lock);
   int port = locked.port;
   ctMutexUnlock(lock);

   void* socket = NULL;
   int result = ctSystemHostSocket(socket, port, 10);
   if (result) {
      ctDebugError("Audition Live Socket Failed to bind port %d", port);
      return result;
   }

   bool run = true;
   while (run) {
      ctMutexLock(lock);
      run = locked.shouldRun;
      ctMutexUnlock(lock);
      ctAuditionLiveSyncProp nextProp = ctAuditionLiveSyncProp();
      int recievedBytes = ctSystemSocketRecv(
        socket, (void*)&nextProp.p, sizeof(ctAuditionLiveSyncProp::Packet));
      if (recievedBytes != sizeof(ctAuditionLiveSyncProp::Packet)) { continue; }
      ctMutexLock(lock);
      nextProp.timestamp = ctGetTimestamp();
      ctMutexUnlock(lock);
      SetProp(nextProp);
      ctMutexLock(lock);
      for (size_t i = 0; i < locked.outgoing.Count(); i++) {
         ctSystemSocketSend(
           socket, &locked.outgoing[i].p, sizeof(ctAuditionLiveSyncProp::Packet));
      }
      locked.outgoing.Clear();
      ctMutexUnlock(lock);
   }
   ctSystemCloseSocket(socket);
   ctDebugLog("Audition Live Socket Shutdown");
   return 0;
}