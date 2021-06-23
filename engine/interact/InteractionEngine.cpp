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

#include "InteractionEngine.hpp"
#include "backends/BackendManifest.hpp"

ctResults ctInteractionEngine::Startup() {
   ZoneScoped;
   ctToggleInteractBackend("SdlGamepad", true);
   ctToggleInteractBackend("SdlKeyboardMouse", true);
   CT_RETURN_FAIL(ctStartAndRetrieveInteractBackends(Engine, pBackends));
   for (int i = 0; i < pBackends.Count(); i++) {
      pBackends[i]->Register(Directory);
   }
   return CT_SUCCESS;
}

ctResults ctInteractionEngine::Shutdown() {
   ZoneScoped;
   return ctShutdownInteractBackends();
}

ctResults ctInteractionEngine::PumpInput() {
   ZoneScoped;
   for (int i = 0; i < pBackends.Count(); i++) {
      pBackends[i]->Update(Directory);
   }
   return CT_SUCCESS;
}

ctResults ctInteractionEngine::DebugImGui() {
   ZoneScoped;
   for (int i = 0; i < pBackends.Count(); i++) {
      pBackends[i]->DebugImGui();
   }
   return CT_SUCCESS;
}

float ctInteractionEngine::GetSignal(ctInteractPath& path) {
   ctInteractNode* pNode;
   CT_RETURN_ON_FAIL(Directory.GetNode(path, pNode), 0.0f);
   CT_RETURN_ON_NULL(pNode->pData, 0.0f);
   if (pNode->type == CT_INTERACT_NODETYPE_SCALAR) {
      return *(float*)pNode->pData;
   } else if (pNode->type == CT_INTERACT_NODETYPE_BOOL) {
      return *(bool*)pNode->pData ? 1.0f : 0.0f;
   }
   return 0.0f;
}

ctInteractPath::ctInteractPath() {
   memset(str, 0, CT_MAX_INTERACT_PATH_SIZE);
}

ctInteractPath::ctInteractPath(const char* ptr, size_t count) {
   memset(str, 0, CT_MAX_INTERACT_PATH_SIZE);
   strncpy(str,
           ptr,
           count > CT_MAX_INTERACT_PATH_SIZE - 1 ? CT_MAX_INTERACT_PATH_SIZE - 1 : count);
}

ctInteractPath::ctInteractPath(const char* ptr) {
   *this = ctInteractPath(ptr, strlen(ptr));
}

ctInteractPath::ctInteractPath(const ctStringUtf8& ctStr) {
   *this = ctInteractPath(ctStr.CStr());
}

ctResults ctInteractDirectorySystem::AddNode(ctInteractNode& node) {
   ZoneScoped;
   uint64_t hash = ctxxHash64(node.path.str);
   ctAssert(!nodes.Exists(hash)); /* Catch duplicate on debug builds */
   CT_RETURN_ON_NULL(nodes.Insert(hash, node), CT_FAILURE_INVALID_PARAMETER);
   return CT_SUCCESS;
}

ctResults ctInteractDirectorySystem::RemoveNode(ctInteractPath& path) {
   ZoneScoped;
   ctInteractNode* result;
   CT_RETURN_FAIL(GetNode(path, result, true));
   if (result->type == CT_INTERACT_NODETYPE_MERGER) {
      if (result->pData) { delete result->pData; }
   }
   nodes.Remove(ctxxHash64(path.str));
   return CT_SUCCESS;
}

ctResults ctInteractDirectorySystem::SetNodeAccessible(ctInteractPath& path,
                                                       bool accessible) {
   ZoneScoped;
   ctInteractNode* result;
   CT_RETURN_FAIL(GetNode(path, result, true));
   result->accessible = accessible;
   return CT_SUCCESS;
}

ctResults ctInteractDirectorySystem::GetNode(ctInteractPath& path,
                                             ctInteractNode*& pOutNode,
                                             bool forceAccess) {
   ZoneScoped;
   ctInteractNode* result = nodes.FindPtr(ctxxHash64(path.str));
   CT_RETURN_ON_NULL(result, CT_FAILURE_DATA_DOES_NOT_EXIST);
   ctAssert(result->path == path);
   CT_RETURN_ON_UNTRUE(result->path == path, CT_FAILURE_CORRUPTED_CONTENTS);
   if (!result->accessible && !forceAccess) { return CT_FAILURE_FILE_INACCESSIBLE; }
   pOutNode = result;
   return CT_SUCCESS;
}

void ctInteractDirectorySystem::LogContents() {
   for (auto it = nodes.GetIterator(); it; it++) {
      ctDebugLog("%" PRIu64 ": %s", it.Key(), it.Value().path.str);
   }
}
