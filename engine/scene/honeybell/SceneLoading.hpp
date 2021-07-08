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

namespace ctHoneybell {

typedef class SceneLoadCommandBufferI* SceneLoadCommandBuffer;

/* Create empty command buffer */
ctResults CT_API SceneLoadCommandBufferCreate(SceneLoadCommandBuffer* pBuffer);
/* Loads map command from binary blob */
ctResults CT_API SceneLoadCommandBufferCreateFromBinary(SceneLoadCommandBuffer* pBuffer,
                                                        const void* data,
                                                        size_t size);
/* Destroy command buffer */
void CT_API SceneLoadCommandBufferDestroy(SceneLoadCommandBuffer buffer);

/* Begin command buffer recording */
ctResults CT_API SceneLoadCommandBufferBegin(SceneLoadCommandBuffer buffer);
/* End command buffer recording */
ctResults CT_API SceneLoadCommandBufferEnd(SceneLoadCommandBuffer buffer);
/* End command buffer recording and invalidate */
void CT_API SceneLoadCommandBufferAbort(SceneLoadCommandBuffer buffer);

void CT_API SceneLoadCmdLog(SceneLoadCommandBuffer cmd, const char* text);

void CT_API SceneLoadCmdOpenSection(SceneLoadCommandBuffer cmd, const char* section);

void CT_API SceneLoadCmdSetBool(SceneLoadCommandBuffer cmd, const char* name, bool value);
void CT_API SceneLoadCmdSetScalar(SceneLoadCommandBuffer cmd,
                                  const char* name,
                                  double value);
void CT_API SceneLoadCmdSetVector2(SceneLoadCommandBuffer cmd,
                                   const char* name,
                                   ctVec2 value);
void CT_API SceneLoadCmdSetVector3(SceneLoadCommandBuffer cmd,
                                   const char* name,
                                   ctVec3 value);
void CT_API SceneLoadCmdSetVector4(SceneLoadCommandBuffer cmd,
                                   const char* name,
                                   ctVec4 value);
void CT_API SceneLoadCmdSetString(SceneLoadCommandBuffer cmd,
                                  const char* name,
                                  const char* value);

void CT_API SceneLoadCmdLoadBarrier(SceneLoadCommandBuffer cmd);
void CT_API SceneLoadCmdSpawnStageGLTF(SceneLoadCommandBuffer cmd,
                                       const char* fileName,
                                       const char* mapPathBase,
                                       ctVec3 pos,
                                       ctVec3 eulerXYZ);
void CT_API SceneLoadCmdSpawnToy(SceneLoadCommandBuffer cmd,
                                 const char* prefabPath,
                                 const char* mapPath,
                                 ctVec3 pos,
                                 ctVec3 eulerXYZ,
                                 const char* spawnCommand);
void CT_API SceneLoadCmdSpawnToyRelative(SceneLoadCommandBuffer cmd,
                                         const char* prefabPath,
                                         const char* mapPath,
                                         const char* referencePath,
                                         ctVec3 pos,
                                         ctVec3 eulerXYZ,
                                         const char* spawnCommand);
void CT_API SceneLoadCmdPruneToy(SceneLoadCommandBuffer cmd, const char* path);
void CT_API SceneLoadCmdPruneTag(SceneLoadCommandBuffer cmd, const char* tag);

ctResults CT_API SceneLoadCommandBufferExecute(SceneLoadCommandBuffer buffer,
                                               class Scene* scene);
}