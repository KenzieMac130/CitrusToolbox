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
#include "core/ModuleBase.hpp"
#include "DeviceBackendLayer.hpp"

#if CITRUS_INCLUDE_AUDITION
#include "audition/HotReloadDetection.hpp"
#endif

/* --------------------------- Virtual Directory --------------------------- */

enum ctInteractionNodeType {
   CT_INTERACT_NODETYPE_NULL = 0,
   CT_INTERACT_NODETYPE_SCALAR = 1,      /* Data points to a float */
   CT_INTERACT_NODETYPE_BOOL = 2,        /* Data points to a boolean */
   CT_INTERACT_NODETYPE_BINDING = 127,   /* Data points to a binding structure */
   CT_INTERACT_NODETYPE_ACTIONSET = 128, /* Data points to an action set */
   CT_INTERACT_NODETYPE_MAX = UINT8_MAX
};

struct CT_API ctInteractPath {
   ctInteractPath();
   ctInteractPath(const char* ptr, size_t count);
   ctInteractPath(const char* ptr);
   ctInteractPath(const ctStringUtf8& ctStr);
   bool operator==(const ctInteractPath other) {
      return ctCStrEql(str, other.str);
   }
   char str[CT_MAX_INTERACT_PATH_SIZE];
};

struct CT_API ctInteractNode {
   inline ctInteractNode() {
      path = ctInteractPath();
      type = CT_INTERACT_NODETYPE_NULL;
      accessible = true;
      pData = NULL;
   }
   float GetScalar();
   bool SetScalar(float value);
   class ctInteractBinding* GetAsBinding();
   class ctInteractActionSet* GetAsActionSet();
   ctInteractPath path;
   ctInteractionNodeType type;
   bool accessible;
   void* pData;
};

class CT_API ctInteractDirectorySystem {
public:
   ~ctInteractDirectorySystem();
   ctResults CreateActionSetsFromFile(ctFile& file);
   ctResults CreateBindingsFromFile(ctFile& file);
   ctResults Update();
   ctResults AddNode(ctInteractNode& node);
   ctResults RemoveNode(ctInteractPath& path);
   void EnableActionSet(ctInteractPath& path);
   void DisableActionSet(ctInteractPath& path);
   ctResults SetNodeAccessible(ctInteractPath& path, bool accessible);
   ctResults
   GetNode(ctInteractPath& path, ctInteractNode*& pOutNode, bool forceAccess = false);
   float GetSignal(ctInteractPath& path);
   void FireActions(void (*callback)(const char* path, float value, void* user),
                    void* userdata = NULL);
   void LogContents();
   void DebugImGui();
   void _ReloadClear();

#if CITRUS_INCLUDE_AUDITION
   ctHotReloadCategory configHotReload;
#endif

private:
   ctDynamicArray<ctInteractPath> activeActionSets;
   ctHashTable<ctInteractNode, uint64_t> nodes;
};

/* --------------------------- Bindings --------------------------- */

struct ctInteractBindingEntry {
   ctInteractPath path;
   float scale;
   bool required;
};

class CT_API ctInteractBinding {
public:
   float value;
   void Process(ctInteractDirectorySystem& dir);
   ctDynamicArray<ctInteractBindingEntry> inputs;
   ctInteractPath output;
};

/* --------------------------- Action Sets --------------------------- */
class CT_API ctInteractActionSet {
public:
   ctDynamicArray<ctInteractPath> bindings;
   ctDynamicArray<ctInteractPath> actions;
   ctDynamicArray<float> actionOutputs;
};

/* ---------------------------------- Engine ---------------------------------- */

class CT_API ctInteractionEngine : public ctModuleBase {
public:
   ctResults Startup() final;
   ctResults Shutdown() final;

   ctResults RegisterAll();

   ctResults PumpInput();
   void DebugImGui();

   ctInteractDirectorySystem Directory;
   bool isFrameActive;

protected:
   ctDynamicArray<class ctInteractAbstractBackend*> pBackends;
};