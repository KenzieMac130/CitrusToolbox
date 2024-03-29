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

#pragma once

#include "utilities/Common.h"
#include "SpaceBase.hpp"

class CT_API ctAuditionSpaceHexViewer : public ctAuditionSpaceBase {
public:
   ctAuditionSpaceHexViewer(const char* filePath);
   ~ctAuditionSpaceHexViewer();

   virtual const char* GetTabName();
   virtual const char* GetWindowName();
   virtual void OnGui(ctAuditionSpaceContext& ctx);

public:
   ctStringUtf8 windowName;
   struct MemoryEditor* pMemEditor;
   ctDynamicArray<uint8_t> bytes;
};