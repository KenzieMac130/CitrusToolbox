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

/* ---------------------------------- Virtual Directory ----------------------------------
 */

enum ctInteractionNodeType {
   CT_INTERACT_NODETYPE_NULL = 0,
   CT_INTERACT_NODETYPE_SCALAR = 1, /* Data points to a float */
   CT_INTERACT_NODETYPE_BOOL = 2,   /* Data points to a boolean */
   CT_INTERACT_NODETYPE_MERGER = 8, /* Data points to a value merger structure */
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
   ctInteractPath path;
   ctInteractionNodeType type;
   bool accessible;
   void* pData;
};

class CT_API ctInteractDirectorySystem {
public:
   ctResults AddNode(ctInteractNode& node);
   ctResults RemoveNode(ctInteractPath& path);
   ctResults SetNodeAccessible(ctInteractPath& path, bool accessible);
   ctResults
   GetNode(ctInteractPath& path, ctInteractNode*& pOutNode, bool forceAccess = false);
   void LogContents();

private:
   ctHashTable<ctInteractNode, uint64_t> nodes;
};

/* ---------------------------------- Value Merger ---------------------------------- */

class CT_API ctInteractMergeOp {
   bool required;
   ctInteractPath sourcePath;
};

class CT_API ctInteractMerger {
public:
   ctInteractPath targetPath;
   ctDynamicArray<ctInteractMergeOp> mergeOps;
};

/* ---------------------------------- Engine ---------------------------------- */

class CT_API ctInteractionEngine : public ctModuleBase {
public:
   ctResults Startup() final;
   ctResults Shutdown() final;

   ctResults PumpInput();
   ctResults DebugImGui();

   float GetSignal(ctInteractPath& path);

   ctInteractDirectorySystem Directory;

protected:
   ctDynamicArray<class ctInteractAbstractBackend*> pBackends;
};