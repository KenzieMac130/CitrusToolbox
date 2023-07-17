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

#include "Config.h"

#ifdef __cplusplus
extern "C" {
#endif

#if CITRUS_STATIC_GAME
#define GAME_LAYER_API
#endif
#if !defined(GAME_LAYER_API)
#if defined(_MSC_VER)
#define GAME_LAYER_API __declspec(dllexport)
#endif
#elif defined(__GNUC__)
#define GAME_LAYER_API __attribute__((visibility("default")))
#else
#define GAME_LAYER_API
#endif

GAME_LAYER_API int ctGameAPIStartup(void* pEngine);
GAME_LAYER_API int ctGameAPIShutdown(void);

#ifdef __cplusplus
}
#endif