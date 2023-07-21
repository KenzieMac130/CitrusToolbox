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
#include "core/ModuleBase.hpp"
#include "DeviceBackendLayer.hpp"

#if CITRUS_INCLUDE_AUDITION
#include "audition/HotReloadDetection.hpp"
#endif

/* Simple Input API */
float ctGetSignal(const char* path); /* todo: add local player index */
bool ctGetButton(const char* path);

/* todo: paths should only be manipulated by backends (changing values) or lua (which will be another backend)*/

/* --------------------------- Virtual Directory --------------------------- */

enum ctInteractionNodeType {
   CT_INTERACT_NODETYPE_NULL = 0,
   CT_INTERACT_NODETYPE_SCALAR = 1,      /* Data points to a float */ /* todo: float pointer */
   CT_INTERACT_NODETYPE_BOOL = 2,        /* Data points to a boolean */ /* todo: bool pointer */
   CT_INTERACT_NODETYPE_BINDING = 127,   /* Data points to a binding structure */ /* todo: remove */
   CT_INTERACT_NODETYPE_ACTIONSET = 128, /* Data points to an action set */ /* todo: remove */
   /* todo: CT_INTERACT_NODETYPE_SCRIPT_SCALAR (Data is a double) */
   /* todo: CT_INTERACT_NODETYPE_SCRIPT_STRING (Data points to a string) */
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

/* todo: make object oriented class with one node per type */
struct CT_API ctInteractNode {
   inline ctInteractNode() {
      path = ctInteractPath();
      type = CT_INTERACT_NODETYPE_NULL;
      accessible = true;
      pData = NULL;
   }
   float GetScalar();
   bool SetScalar(float value);
   class ctInteractBinding* GetAsBinding(); /* todo: remove */
   class ctInteractActionSet* GetAsActionSet(); /* todo: remove */
   ctInteractPath path;
   ctInteractionNodeType type;
   bool accessible; /* todo: remove*/
   void* pData; /* todo: replace with proper get/set behind virtuals */
};

/* todo: one per player */
class CT_API ctInteractDirectorySystem {
public:
   ~ctInteractDirectorySystem();
   ctResults CreateActionSetsFromFile(ctFile& file); /* todo: run action set lua script */
   ctResults CreateBindingsFromFile(ctFile& file); /* todo: feeds user settings json to script */
   ctResults Update();
   ctResults AddNode(ctInteractNode& node);
   ctResults RemoveNode(ctInteractPath& path);
   void EnableActionSet(ctInteractPath& path); /* todo: remove, now handled by params */
   void DisableActionSet(ctInteractPath& path); /* todo: remove, now handled by params */
   ctResults SetNodeAccessible(ctInteractPath& path, bool accessible); /* todo: remove */
   ctResults
   GetNode(ctInteractPath& path, ctInteractNode*& pOutNode, bool forceAccess = false); /* todo: remove force access*/
   float GetSignal(ctInteractPath& path);
/* todo: request device (requests a device be mapped to the directory)*/
/* */

   void LogContents();
   void DebugImGui();
   void _ReloadClear();

#if CITRUS_INCLUDE_AUDITION
   ctHotReloadCategory configHotReload;
#endif

private:
   ctDynamicArray<ctInteractPath> activeActionSets; /* todo: remove */
   ctHashTable<ctInteractNode, uint64_t> nodes; /* todo: make pointer objects */
};

/* --------------------------- Bindings --------------------------- */

/* todo: remove and make the responsibility of Lua */
struct ctInteractBindingEntry {
   ctInteractPath path;
   float scale = 1.0f;
   float clampMin = -FLT_MAX;
   float clampMax = FLT_MAX;
   float deadzone;
   bool required;
   bool invert;
};

class CT_API ctInteractBinding {
public:
   float value;
   void Process(ctInteractDirectorySystem& dir);
   ctDynamicArray<ctInteractBindingEntry> inputs;
   ctInteractPath output;
};

/* --------------------------- Actions --------------------------- */

/* todo: remove and make the responsibility of Lua */
struct ctInteractActionEntry {
   ctInteractPath path;
   ctInteractPath velocityPath;
};

/* --------------------------- Action Sets --------------------------- */

/* todo: remove and make the responsibility of Lua */
class CT_API ctInteractActionSet {
public:
   ctDynamicArray<ctInteractPath> bindings;
   ctDynamicArray<ctInteractActionEntry> actions;
   ctDynamicArray<float> actionOutputs;
   ctDynamicArray<float> actionPrevious;
   ctDynamicArray<float> actionVelocities;
};

/* ---------------------------------- Engine ---------------------------------- */

class CT_API ctInteractionEngine : public ctModuleBase {
public:
   ctInteractionEngine(bool shared);
   ctResults Startup() final;
   ctResults Shutdown() final;
   const char* GetModuleName() final;

   ctResults RegisterAll();

   ctResults PumpInput();
   virtual void DebugUI(bool useGizmos);

   ctInteractDirectorySystem Directory; /* todo: one per player */
   bool isFrameActive; /* todo: move to directory */

protected:
   ctDynamicArray<class ctInteractAbstractBackend*> pBackends;
};