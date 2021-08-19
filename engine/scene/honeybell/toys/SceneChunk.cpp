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

#include "SceneChunk.hpp"

ctHoneybell::SceneChunk::SceneChunk(ConstructContext& ctx) : ToyBase(ctx) {
   void* luaLumpData = NULL;
   int32_t luaLumpSize = 0;
   ctWADFindLump(&ctx.prefab.wadReader, "LUA", &luaLumpData, &luaLumpSize);
   sceneScript.LoadFromBuffer((const char*)luaLumpData, luaLumpSize, "SceneScript");
   sceneScript.Startup(false);
   sceneScript.OpenEngineLibrary("scene");
   sceneScript.CallFunction("OnConstruct", "");
}

ctHoneybell::SceneChunk::~SceneChunk() {
   sceneScript.CallFunction("OnDestruct", "");
   sceneScript.Shutdown();
}

ctResults ctHoneybell::SceneChunk::OnBegin(BeginContext& ctx) {
   ctx.canFrameUpdate = true;
   ctx.canTickSerial = true;
   sceneScript.CallFunction("OnBegin", "");
   BeginComponents(ctx);
   return CT_SUCCESS;
}

ctResults ctHoneybell::SceneChunk::OnTickSerial(TickContext& ctx) {
   sceneScript.CallFunction("OnTick", "");
   return CT_SUCCESS;
}

ctResults ctHoneybell::SceneChunk::OnFrameUpdate(FrameUpdateContext& ctx) {
   sceneScript.CallFunction("OnFrameUpdate", "");
   return CT_SUCCESS;
}
