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

#include "gpu/Device.h"

struct ctGPUDeviceBase {
   /* Extern */
   struct SDL_Window* pMainWindow;
   ctGPUOpenCacheFileFn fpOpenCacheFileCallback;
   void* pCacheCallbackCustomData;
   ctGPUOpenAssetFileFn fpOpenAssetFileCallback;
   void* pAssetCallbackCustomData;

   inline ctFile OpenCacheFile(const char* path, ctFileOpenMode openMode) const {
      ctFile result = ctFile();
      fpOpenCacheFileCallback(&result, path, (int)openMode, pCacheCallbackCustomData);
      return result;
   }
   inline ctFile OpenAssetFile(ctGPUAssetIdentifier* pAssetIdentifier) const {
      ctFile result = ctFile();
      fpOpenAssetFileCallback(&result, pAssetIdentifier, pAssetCallbackCustomData);
      return result;
   }
};