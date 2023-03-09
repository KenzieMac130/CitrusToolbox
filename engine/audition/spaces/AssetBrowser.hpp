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

class CT_API ctAuditionSpaceAssetBrowser : public ctAuditionSpaceBase {
public:
   ctAuditionSpaceAssetBrowser();
   ~ctAuditionSpaceAssetBrowser();

   virtual const char* GetTabName();
   virtual const char* GetWindowName();

   virtual void OnGui(ctAuditionSpaceContext& ctx);

private:
   ctStringUtf8 baseFolder = "";
   ctStringUtf8 relativeFolder = "";
   void CurrentFromRelative(ctStringUtf8& currentFolder);

   struct DirectoryEntry {
      bool passedSearch;
      bool isFile;
      size_t size;
      time_t date;
      char type[32];
      char name[CT_MAX_FILE_PATH_LENGTH];
      char path[CT_MAX_FILE_PATH_LENGTH];

      static int CmpName(const DirectoryEntry* a, const DirectoryEntry* b);
      static int CmpDate(const DirectoryEntry* a, const DirectoryEntry* b);
      static int CmpType(const DirectoryEntry* a, const DirectoryEntry* b);
      static int CmpSize(const DirectoryEntry* a, const DirectoryEntry* b);
   };
   ctDynamicArray<DirectoryEntry> directories;
   bool directoryValid;

   void RefreshDirectories(const char* folder);

   void SortByName();
   void SortByDate();
   void SortByType();
   void SortBySize();

   ctStringUtf8 searchKeyword = "";
   void ApplySearch();
};