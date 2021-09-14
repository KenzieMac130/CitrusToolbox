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

/* Todo: Signal filtering (inverter, pulse, on-release, smoothing, etc)*/
/* Todo: Input buffering to avoid dropped inputs at high framerates */

ctResults ctInteractionEngine::Startup() {
   ZoneScoped;
#if CITRUS_INCLUDE_AUDITION
   Engine->HotReload->RegisterAssetCategory(&Directory.configHotReload);
#endif
   ctToggleInteractBackend("SdlGamepad", true);
   ctToggleInteractBackend("SdlKeyboardMouse", true);
   CT_RETURN_FAIL(ctStartAndRetrieveInteractBackends(Engine, pBackends));
   RegisterAll();
   return CT_SUCCESS;
}

ctResults ctInteractionEngine::Shutdown() {
   ZoneScoped;
   return ctShutdownInteractBackends();
}

ctResults ctInteractionEngine::RegisterAll() {
   ctFile file;
   Engine->FileSystem->OpenAssetFile(file, "input/actions.json", CT_FILE_OPEN_READ_TEXT);
   Directory.CreateActionSetsFromFile(file);
#if CITRUS_INCLUDE_AUDITION
   Directory.configHotReload.RegisterPath("input/actions.json");
#endif
   file.Close();
   for (int i = 0; i < pBackends.Count(); i++) {
      pBackends[i]->Register(Directory);
   }
   return CT_SUCCESS;
}

ctResults ctInteractionEngine::PumpInput() {
   ZoneScoped;
#if CITRUS_INCLUDE_AUDITION
   if (Directory.configHotReload.isContentUpdated()) {
      Directory.configHotReload.ClearChanges();
      Directory._ReloadClear();
      RegisterAll();
      ctDebugLog("Reloaded Interact Configs...");
   }
#endif
   isFrameActive = true;
   for (int i = 0; i < pBackends.Count(); i++) {
      pBackends[i]->Update(Directory);
   }
   Directory.Update();
   return CT_SUCCESS;
}

void ctInteractionEngine::DebugImGui() {
   ZoneScoped;
   for (int i = 0; i < pBackends.Count(); i++) {
      pBackends[i]->DebugImGui();
   }
   Directory.DebugImGui();
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
   if (!ptr) {
      memset(str, 0, CT_MAX_INTERACT_PATH_SIZE);
      return;
   }
   *this = ctInteractPath(ptr, strlen(ptr));
}

ctInteractPath::ctInteractPath(const ctStringUtf8& ctStr) {
   *this = ctInteractPath(ctStr.CStr());
}

ctInteractDirectorySystem::~ctInteractDirectorySystem() {
   for (auto it = nodes.GetIterator(); it; it++) {
      if (it.Value().type == CT_INTERACT_NODETYPE_BINDING ||
          it.Value().type == CT_INTERACT_NODETYPE_ACTIONSET) {
         if (it.Value().pData) { delete it.Value().pData; }
      }
   }
}

ctResults ctInteractDirectorySystem::CreateActionSetsFromFile(ctFile& file) {
   ctDebugLog("Loading Action Sets...");
   size_t fileSize = file.GetFileSize();
   char* fileData = (char*)ctMalloc(fileSize + 1);
   memset(fileData, 0, fileSize + 1);
   file.ReadRaw(fileData, 1, fileSize);
   ctJSONReader json;
   CT_RETURN_FAIL_CLEAN(json.BuildJsonForPtr(fileData, fileSize), ctFree(fileData);
                        ctDebugError("ACTION SET FILE INVALID!");)
   ctJSONReadEntry jsonRoot = ctJSONReadEntry();
   json.GetRootEntry(jsonRoot);

   int numSets = jsonRoot.GetObjectEntryCount();
   for (int i = 0; i < numSets; i++) {
      ctStringUtf8 setName;
      ctJSONReadEntry jsonActionSet = ctJSONReadEntry();
      jsonRoot.GetObjectEntry(i, jsonActionSet, &setName);

      /* Create Action Set */
      ctInteractNode node = ctInteractNode();
      ctInteractActionSet* pActionSet = new ctInteractActionSet();
      node.type = CT_INTERACT_NODETYPE_ACTIONSET;
      node.pData = pActionSet;
      node.path = setName;
      AddNode(node);
      EnableActionSet(ctInteractPath(setName.CStr()));

      const int numActions = (size_t)jsonActionSet.GetObjectEntryCount();
      pActionSet->actionOutputs.Resize(numActions);
      pActionSet->actionOutputs.Memset(0);
      pActionSet->actionPrevious.Resize(numActions);
      pActionSet->actionPrevious.Memset(0);
      pActionSet->actionVelocities.Resize(numActions);
      pActionSet->actionVelocities.Memset(0);
      for (int j = 0; j < numActions; j++) {
         ctStringUtf8 actionPath;

         ctStringUtf8 dispatchPhase;
         ctJSONReadEntry jsonAction = ctJSONReadEntry();
         jsonActionSet.GetObjectEntry(j, jsonAction, &actionPath);
         if (actionPath.isEmpty()) { continue; }
         ctStringUtf8 actionVelocityPath;
         actionVelocityPath.Printf(1024, "%s/velocity", actionPath.CStr());
         ctJSONReadEntry jsonActionDispatch = ctJSONReadEntry();
         jsonAction.GetObjectEntry("dispatch", jsonActionDispatch);
         jsonActionDispatch.GetString(dispatchPhase);

         ctInteractNode node = ctInteractNode();
         node.type = CT_INTERACT_NODETYPE_SCALAR;
         node.pData = &pActionSet->actionOutputs[j];
         node.path = actionPath;
         AddNode(node);
         node.pData = &pActionSet->actionVelocities[j];
         node.path = actionVelocityPath;
         AddNode(node);

         ctInteractActionEntry actionEntry = ctInteractActionEntry();
         actionEntry.path = actionPath;
         actionEntry.velocityPath = actionVelocityPath;
         if (dispatchPhase == "tick") {
            actionEntry.phase = CT_INTERACT_ACTIONDISPATCH_TICK;
         } else if (dispatchPhase == "update") {
            actionEntry.phase = CT_INTERACT_ACTIONDISPATCH_UPDATE;
         }
         pActionSet->actions.Append(actionEntry);
      }
   }
   ctFree(fileData);
   return CT_SUCCESS;
}

ctResults ctInteractDirectorySystem::CreateBindingsFromFile(ctFile& file) {
   size_t fileSize = file.GetFileSize();
   char* fileData = (char*)ctMalloc(fileSize + 1);
   memset(fileData, 0, fileSize + 1);
   file.ReadRaw(fileData, 1, fileSize);
   ctJSONReader json;
   CT_RETURN_FAIL_CLEAN(json.BuildJsonForPtr(fileData, fileSize), ctFree(fileData);
                        ctDebugError("BINDING FILE INVALID!");)
   ctJSONReadEntry jsonRoot = ctJSONReadEntry();
   json.GetRootEntry(jsonRoot);

   int numBinds = jsonRoot.GetObjectEntryCount();
   for (int i = 0; i < numBinds; i++) {
      ctStringUtf8 bindName;
      ctJSONReadEntry jsonBind = ctJSONReadEntry();
      jsonRoot.GetObjectEntry(i, jsonBind, &bindName);

      ctInteractNode node = ctInteractNode();
      ctInteractBinding* pBinding = new ctInteractBinding();
      node.type = CT_INTERACT_NODETYPE_BINDING;
      node.pData = pBinding;
      node.path = bindName;
      AddNode(node);

      ctJSONReadEntry output = ctJSONReadEntry();
      ctJSONReadEntry inputs = ctJSONReadEntry();
      jsonBind.GetObjectEntry("output", output);
      jsonBind.GetObjectEntry("inputs", inputs);

      ctStringUtf8 outputPath;
      output.GetString(outputPath);
      pBinding->output = outputPath;
      ctInteractNode* pSetNode = NULL;
      outputPath.FilePathPop();
      GetNode(ctInteractPath(outputPath), pSetNode);
      if (pSetNode) {
         ctInteractActionSet* actionSet = pSetNode->GetAsActionSet();
         if (actionSet) { actionSet->bindings.Append(node.path); }
      }

      int numInputs = inputs.GetArrayLength();
      for (int j = 0; j < numInputs; j++) {
         ctInteractBindingEntry bindEntry = ctInteractBindingEntry();
         ctJSONReadEntry jsonEntry = ctJSONReadEntry();
         ctJSONReadEntry path = ctJSONReadEntry();
         ctJSONReadEntry scale = ctJSONReadEntry();
         ctJSONReadEntry required = ctJSONReadEntry();
         ctJSONReadEntry invert = ctJSONReadEntry();
         ctJSONReadEntry min = ctJSONReadEntry();
         ctJSONReadEntry max = ctJSONReadEntry();
         inputs.GetArrayEntry(j, jsonEntry);
         jsonEntry.GetObjectEntry("path", path);
         jsonEntry.GetObjectEntry("scale", scale);
         jsonEntry.GetObjectEntry("required", required);
         jsonEntry.GetObjectEntry("invert", invert);
         jsonEntry.GetObjectEntry("min", min);
         jsonEntry.GetObjectEntry("max", max);

         ctStringUtf8 pathStr;
         path.GetString(pathStr);
         bindEntry.path = pathStr.CStr();
         scale.GetNumber(bindEntry.scale);
         min.GetNumber(bindEntry.clampMin);
         max.GetNumber(bindEntry.clampMax);
         required.GetBool(bindEntry.required);
         invert.GetBool(bindEntry.invert);
         pBinding->inputs.Append(bindEntry);
      }
   }
   ctFree(fileData);
   return CT_SUCCESS;
}

ctResults ctInteractDirectorySystem::Update() {
   ZoneScoped;
   for (size_t i = 0; i < activeActionSets.Count(); i++) {
      ctInteractNode* pSetNode = NULL;
      GetNode(activeActionSets[i], pSetNode);
      if (!pSetNode) { continue; }
      ctInteractActionSet* pActionSet = pSetNode->GetAsActionSet();
      if (!pActionSet) { continue; }
      for (size_t j = 0; j < pActionSet->bindings.Count(); j++) {
         ctInteractNode* pBindNode = NULL;
         GetNode(pActionSet->bindings[j], pBindNode);
         if (!pBindNode) { continue; }
         ctInteractBinding* pBinding = pBindNode->GetAsBinding();
         if (!pBinding) { continue; }
         pBinding->Process(*this);
      }
      ctAssert(pActionSet->actionOutputs.Count() == pActionSet->actionOutputs.Count());
      ctAssert(pActionSet->actionOutputs.Count() == pActionSet->actionVelocities.Count());
      for (size_t j = 0; j < pActionSet->actionOutputs.Count(); j++) {
         pActionSet->actionVelocities[j] =
           pActionSet->actionOutputs[j] - pActionSet->actionPrevious[j];
         pActionSet->actionPrevious[j] = pActionSet->actionOutputs[j];
      }
   }
   return CT_SUCCESS;
}

ctResults ctInteractDirectorySystem::AddNode(ctInteractNode& node) {
   ZoneScoped;
   ctAssert(!ctCStrNEql(node.path.str, "", CT_MAX_INTERACT_PATH_SIZE));
   uint64_t hash = ctXXHash64(node.path.str);
   ctAssert(!nodes.Exists(hash)); /* Catch duplicate on debug builds */
   CT_RETURN_ON_NULL(nodes.Insert(hash, node), CT_FAILURE_INVALID_PARAMETER);
   return CT_SUCCESS;
}

ctResults ctInteractDirectorySystem::RemoveNode(ctInteractPath& path) {
   ZoneScoped;
   ctInteractNode* result;
   CT_RETURN_FAIL(GetNode(path, result, true));
   if (result->type == CT_INTERACT_NODETYPE_BINDING ||
       result->type == CT_INTERACT_NODETYPE_ACTIONSET) {
      if (result->pData) { delete result->pData; }
   }
   nodes.Remove(ctXXHash64(path.str));
   return CT_SUCCESS;
}

void ctInteractDirectorySystem::EnableActionSet(ctInteractPath& path) {
   if (!activeActionSets.Exists(path)) { activeActionSets.Append(path); }
}

void ctInteractDirectorySystem::DisableActionSet(ctInteractPath& path) {
   while (activeActionSets.Exists(path)) {
      activeActionSets.Remove(path);
   }
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
   ctInteractNode* result = nodes.FindPtr(ctXXHash64(path.str));
   CT_RETURN_ON_NULL(result, CT_FAILURE_DATA_DOES_NOT_EXIST);
   ctAssert(result->path == path);
   CT_RETURN_ON_UNTRUE(result->path == path, CT_FAILURE_CORRUPTED_CONTENTS);
   if (!result->accessible && !forceAccess) { return CT_FAILURE_INACCESSIBLE; }
   pOutNode = result;
   return CT_SUCCESS;
}

float ctInteractDirectorySystem::GetSignal(ctInteractPath& path) {
   ctInteractNode* pNode;
   CT_RETURN_ON_FAIL(GetNode(path, pNode), 0.0f);
   return pNode->GetScalar();
}

void ctInteractDirectorySystem::FireActions(ctInteractActionDispatchPhase phase,
                                            void (*callback)(const char* path,
                                                             float value,
                                                             void* user),
                                            void* userdata) {
   ZoneScoped;
   for (size_t i = 0; i < activeActionSets.Count(); i++) {
      ctInteractNode* pSetNode = NULL;
      GetNode(activeActionSets[i], pSetNode);
      ctInteractActionSet* pActionSet = pSetNode->GetAsActionSet();
      if (!pActionSet) { continue; }
      for (size_t j = 0; j < pActionSet->actions.Count(); j++) {
         // if (pActionSet->actions[j].phase != phase) { continue; }
         ctInteractNode* pActionNode = NULL;
         GetNode(pActionSet->actions[j].path, pActionNode);
         if (!pActionNode) { continue; }
         float value = pActionNode->GetScalar();
         if (value) { callback(pActionSet->actions[j].path.str, value, userdata); }
      }
   }
}

void ctInteractDirectorySystem::LogContents() {
   ZoneScoped;
   for (auto it = nodes.GetIterator(); it; it++) {
      ctDebugLog("%" PRIu64 ": %s", it.Key(), it.Value().path.str);
   }
}

#include "imgui/imgui.h"

void ctInteractDirectorySystem::DebugImGui() {
   for (auto it = nodes.GetIterator(); it; it++) {
      if (it.Value().GetScalar()) {
         ImGui::Text("%s: %f", it.Value(), it.Value().GetScalar());
      }
   }
}

void ctInteractDirectorySystem::_ReloadClear() {
   for (auto it = nodes.GetIterator(); it; it++) {
      if (it.Value().type == CT_INTERACT_NODETYPE_BINDING ||
          it.Value().type == CT_INTERACT_NODETYPE_ACTIONSET) {
         if (it.Value().pData) { delete it.Value().pData; }
      }
   }
   nodes.Clear();
}

float ctInteractNode::GetScalar() {
   CT_RETURN_ON_NULL(pData, 0.0f);
   if (type == CT_INTERACT_NODETYPE_SCALAR) {
      return *(float*)pData;
   } else if (type == CT_INTERACT_NODETYPE_BOOL) {
      return *(bool*)pData ? 1.0f : 0.0f;
   } else if (type == CT_INTERACT_NODETYPE_BINDING) {
      return GetAsBinding()->value;
   }
   return 0.0f;
}

bool ctInteractNode::SetScalar(float value) {
   CT_RETURN_ON_NULL(pData, false);
   if (type == CT_INTERACT_NODETYPE_SCALAR) {
      (*(float*)pData) = value;
      return true;
   } else if (type == CT_INTERACT_NODETYPE_BOOL) {
      value ? (*(bool*)pData) = true : (*(bool*)pData) = false;
      return true;
   }
   return false;
}

ctInteractBinding* ctInteractNode::GetAsBinding() {
   if (type == CT_INTERACT_NODETYPE_BINDING) { return (ctInteractBinding*)pData; }
   return NULL;
}

ctInteractActionSet* ctInteractNode::GetAsActionSet() {
   if (type == CT_INTERACT_NODETYPE_ACTIONSET) { return (ctInteractActionSet*)pData; }
   return NULL;
}

void ctInteractBinding::Process(ctInteractDirectorySystem& dir) {
   value = 0.0f;
   for (size_t i = 0; i < inputs.Count(); i++) {
      ctInteractBindingEntry entry = inputs[i];
      ctInteractNode* pNode = NULL;
      dir.GetNode(entry.path, pNode);
      if (!pNode) { continue; }
      float entryValue = pNode->GetScalar();
      ctClamp(entryValue, entry.clampMin, entry.clampMax);
      if (entry.invert) { entryValue = 1.0f - entryValue; }
      if (entry.required) {
         if (i) {
            value *= entry.scale * entryValue;
         } else {
            value = entry.scale * entryValue;
         }
      } else {
         value += entry.scale * entryValue;
      }
   }
   ctInteractNode* pNode = NULL;
   dir.GetNode(output, pNode);
   if (!pNode) { return; }
   pNode->SetScalar(value);
}