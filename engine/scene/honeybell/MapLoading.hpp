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

enum ctMapLoadCmdType {
   CT_MAPLOADCMD_NULL = 0,
   CT_MAPLOADCMD_DEBUG_LOG = 1,
   // RESERVED = 2 - 31
   CT_MAPLOADCMD_OPEN_SECTION = 32,
   CT_MAPLOADCMD_SET_BOOL = 33,
   CT_MAPLOADCMD_SET_SCALAR = 34,
   CT_MAPLOADCMD_SET_VECTOR2 = 35,
   CT_MAPLOADCMD_SET_VECTOR3 = 36,
   CT_MAPLOADCMD_SET_VECTOR4 = 37,
   CT_MAPLOADCMD_SET_STRING = 38,
   // RESERVED = 38 - 127
   CT_MAPLOADCMD_LOAD_BARRIER = 128,
   CT_MAPLOADCMD_SPAWN_STAGE_GLTF = 129,
   CT_MAPLOADCMD_SPAWN_TOY = 130,
   CT_MAPLOADCMD_SPAWN_TOY_RELATIVE = 131,
   CT_MAPLOADCMD_PRUNE_TOY = 132,
   CT_MAPLOADCMD_PRUNE_TAG = 133,
   CT_MAPLOADCMD_MAX = UINT8_MAX
};

typedef class ctMapLoadCommandBufferI* ctMapLoadCommandBuffer;

/* Create empty command buffer */
ctResults CT_API ctMapLoadCommandBufferCreate(ctMapLoadCommandBuffer* pBuffer);
/* Loads map command from binary blob */
ctResults CT_API ctMapLoadCommandBufferCreateFromBinary(ctMapLoadCommandBuffer* pBuffer,
                                           const void* data,
                                           size_t size);
/* Destroy command buffer */
void CT_API ctMapLoadCommandBufferDestroy(ctMapLoadCommandBuffer buffer);

/* Begin command buffer recording */
ctResults CT_API ctMapLoadCommandBufferBegin(ctMapLoadCommandBuffer buffer);
/* End command buffer recording */
ctResults CT_API ctMapLoadCommandBufferEnd(ctMapLoadCommandBuffer buffer);
/* End command buffer recording and invalidate */
void CT_API ctMapLoadCommandBufferAbort(ctMapLoadCommandBuffer buffer);

void CT_API ctMapLoadCmdLog(ctMapLoadCommandBuffer cmd, const char* text);

void CT_API ctMapLoadCmdOpenSection(ctMapLoadCommandBuffer cmd, const char* section);

void CT_API ctMapLoadCmdSetBool(ctMapLoadCommandBuffer cmd, const char* name, bool value);
void CT_API ctMapLoadCmdSetScalar(ctMapLoadCommandBuffer cmd, const char* name, double value);
void CT_API ctMapLoadCmdSetVector2(ctMapLoadCommandBuffer cmd, const char* name, ctVec2 value);
void CT_API ctMapLoadCmdSetVector3(ctMapLoadCommandBuffer cmd, const char* name, ctVec3 value);
void CT_API ctMapLoadCmdSetVector4(ctMapLoadCommandBuffer cmd, const char* name, ctVec4 value);
void CT_API ctMapLoadCmdSetString(ctMapLoadCommandBuffer cmd,
                           const char* name,
                           const char* value);

void CT_API ctMapLoadCmdLoadBarrier(ctMapLoadCommandBuffer cmd);
void CT_API ctMapLoadCmdSpawnStageGLTF(ctMapLoadCommandBuffer cmd,
                                const char* fileName,
                                const char* mapPathBase,
                                ctVec3 pos,
                                ctVec3 eulerXYZ);
void CT_API ctMapLoadCmdSpawnToy(ctMapLoadCommandBuffer cmd,
                          const char* prefabPath,
                          const char* mapPath,
                          ctVec3 pos,
                          ctVec3 eulerXYZ,
                          const char* spawnCommand);
void CT_API ctMapLoadCmdSpawnToyRelative(ctMapLoadCommandBuffer cmd,
                                  const char* prefabPath,
                                  const char* mapPath,
                                  const char* referencePath,
                                  ctVec3 pos,
                                  ctVec3 eulerXYZ,
                                  const char* spawnCommand);
void CT_API ctMapLoadCmdPruneToy(ctMapLoadCommandBuffer cmd, const char* path);
void CT_API ctMapLoadCmdPruneTag(ctMapLoadCommandBuffer cmd, const char* tag);

ctResults CT_API ctMapLoadCommandBufferExecute(ctMapLoadCommandBuffer buffer, void* scene);