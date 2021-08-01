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

typedef uint32_t ctHandle;

struct _ctInternalHandleRep {
   union {
      struct {
         unsigned int idx : 24;
         unsigned int gen : 8;
      };
      ctHandle data;
   };
};

class ctHandleManager {
public:
   inline ctHandleManager() {
      _freeList.Clear();
      _nextOpenIdx = 1; /* 1 is used for hashtable compatibility */
   };
   inline ctHandle GetNewHandle();
   inline void FreeHandle(ctHandle hndl);

private:
   ctDynamicArray<_ctInternalHandleRep> _freeList;
   unsigned int _nextOpenIdx;
};

inline ctHandle ctHandleManager::GetNewHandle() {
   _ctInternalHandleRep result;
   if (_freeList.isEmpty()) {
      result = {_nextOpenIdx, 0};
      _nextOpenIdx++;
   } else {
      result = _freeList.Last();
      result.gen++;
      _freeList.RemoveLast();
   }
   return result.data;
}
inline void ctHandleManager::FreeHandle(ctHandle hndl) {
   _ctInternalHandleRep toInsert;
   toInsert.data = hndl;
   _freeList.Append(toInsert);
}