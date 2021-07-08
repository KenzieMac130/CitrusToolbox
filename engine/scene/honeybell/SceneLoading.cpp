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

#include "SceneLoading.hpp"
#include "Scene.hpp"

namespace ctHoneybell {

enum SceneLoadCmdType {
   HB_SCNLOADCMD_NULL = 0,
   HB_SCNLOADCMD_DEBUG_LOG = 1,
   // RESERVED = 2 - 31
   HB_SCNLOADCMD_OPEN_SECTION = 32,
   HB_SCNLOADCMD_SET_BOOL = 33,
   HB_SCNLOADCMD_SET_SCALAR = 34,
   HB_SCNLOADCMD_SET_VECTOR2 = 35,
   HB_SCNLOADCMD_SET_VECTOR3 = 36,
   HB_SCNLOADCMD_SET_VECTOR4 = 37,
   HB_SCNLOADCMD_SET_STRING = 38,
   // RESERVED = 38 - 127
   HB_SCNLOADCMD_LOAD_BARRIER = 128,
   HB_SCNLOADCMD_SPAWN_STAGE_GLTF = 129,
   HB_SCNLOADCMD_SPAWN_TOY = 130,
   HB_SCNLOADCMD_SPAWN_TOY_RELATIVE = 131,
   HB_SCNLOADCMD_PRUNE_TOY = 132,
   HB_SCNLOADCMD_PRUNE_TAG = 133,
   HB_SCNLOADCMD_MAX = UINT8_MAX
};

struct CT_API SceneLoadCommandHeader {
   SceneLoadCmdType type;
   uint32_t nextCmdBytePos;
};

class SceneLoadCommandBufferI {
public:
   inline SceneLoadCommandBufferI() {
      bytes.Clear();
      canWrite = true;
      recording = false;
   }
   inline SceneLoadCommandBufferI(const void* data, size_t size) {
      bytes.Resize(size + 1);
      bytes.Memset(0); /* Ensure null termination */
      memcpy(bytes.Data(), data, size);
      canWrite = false;
      recording = false;
   }

   inline bool CanWrite() {
      return recording;
   }
   inline ctResults Begin() {
      if (!canWrite) { return CT_FAILURE_NOT_UPDATABLE; }
      recording = true;
      return CT_SUCCESS;
   }
   inline ctResults End() {
      if (!recording) { return CT_FAILURE_NOT_UPDATABLE; }
      bytes.Append(0); /* Ensure null termination */
      recording = false;
      return CT_SUCCESS;
   }
   inline void Abort() {
      bytes.Clear();
      recording = false;
      canWrite = true;
   }

   inline void AddData(void* data, size_t size) {
      ctAssert(CanWrite());
      size_t current = bytes.Count();
      bytes.Resize(current + size);
      memcpy(&bytes[current], data, size);
   }

   inline uint32_t CalcNextOffset(size_t size) {
      return (uint32_t)(bytes.Count() + sizeof(SceneLoadCommandHeader) + size);
   }

   inline void AddCommand(SceneLoadCmdType type) {
      SceneLoadCommandHeader header = {type, CalcNextOffset(0)};
      AddData(&header, sizeof(header));
   }
   inline void AddCommand(SceneLoadCmdType type, void* pData, size_t size) {
      SceneLoadCommandHeader header = {type, CalcNextOffset(size)};
      AddData(&header, sizeof(header));
      AddData(pData, size);
   }
   inline void
   AddCommand(SceneLoadCmdType type, size_t dataCount, void** ppData, size_t* pSizes) {
      size_t total = 0;
      for (size_t i = 0; i < dataCount; i++) {
         total += pSizes[i];
      }
      SceneLoadCommandHeader header = {type, CalcNextOffset(total)};
      AddData(&header, sizeof(header));
      for (size_t i = 0; i < dataCount; i++) {
         AddData(ppData[i], pSizes[i]);
      }
   }

private:
   bool canWrite;
   bool recording;
   ctDynamicArray<uint8_t> bytes;
};

ctResults CT_API SceneLoadCommandBufferCreate(SceneLoadCommandBuffer* pBuffer) {
   *pBuffer = new SceneLoadCommandBufferI();
   return CT_SUCCESS;
}

ctResults CT_API SceneLoadCommandBufferCreateFromBinary(SceneLoadCommandBuffer* pBuffer,
                                                        const void* data,
                                                        size_t size) {
   *pBuffer = new SceneLoadCommandBufferI(data, size);
   return CT_SUCCESS;
}

void CT_API SceneLoadCommandBufferDestroy(SceneLoadCommandBuffer buffer) {
   delete buffer;
}

ctResults CT_API SceneLoadCommandBufferBegin(SceneLoadCommandBuffer buffer) {
   ctAssert(buffer);
   return buffer->Begin();
}

ctResults CT_API SceneLoadCommandBufferEnd(SceneLoadCommandBuffer buffer) {
   ctAssert(buffer);
   return buffer->End();
}

void CT_API SceneLoadCommandBufferAbort(SceneLoadCommandBuffer buffer) {
   ctAssert(buffer);
   buffer->Abort();
}

void CT_API SceneLoadCmdLog(SceneLoadCommandBuffer cmd, const char* text) {
   ctAssert(cmd);
   cmd->AddCommand(HB_SCNLOADCMD_DEBUG_LOG, (void*)text, strlen(text) + 1);
}

void CT_API SceneLoadCmdOpenSection(SceneLoadCommandBuffer cmd, const char* section) {
   ctAssert(cmd);
   cmd->AddCommand(HB_SCNLOADCMD_OPEN_SECTION, (void*)section, strlen(section) + 1);
}

void CT_API SceneLoadCmdSetBool(SceneLoadCommandBuffer cmd,
                                const char* name,
                                bool value) {
   void* data[] = {(void*)name, &value};
   size_t sizes[] = {strlen(name) + 1, sizeof(value)};
   cmd->AddCommand(HB_SCNLOADCMD_SET_BOOL, 2, data, sizes);
   ctAssert(cmd);
}

void CT_API SceneLoadCmdSetScalar(SceneLoadCommandBuffer cmd,
                                  const char* name,
                                  double value) {
   ctAssert(cmd);
   void* data[] = {(void*)name, &value};
   size_t sizes[] = {strlen(name) + 1, sizeof(value)};
   cmd->AddCommand(HB_SCNLOADCMD_SET_SCALAR, 2, data, sizes);
}

void CT_API SceneLoadCmdSetVector2(SceneLoadCommandBuffer cmd,
                                   const char* name,
                                   ctVec2 value) {
   ctAssert(cmd);
   void* data[] = {(void*)name, &value};
   size_t sizes[] = {strlen(name) + 1, sizeof(value)};
   cmd->AddCommand(HB_SCNLOADCMD_SET_VECTOR2, 2, data, sizes);
}

void CT_API SceneLoadCmdSetVector3(SceneLoadCommandBuffer cmd,
                                   const char* name,
                                   ctVec3 value) {
   ctAssert(cmd);
   void* data[] = {(void*)name, &value};
   size_t sizes[] = {strlen(name) + 1, sizeof(value)};
   cmd->AddCommand(HB_SCNLOADCMD_SET_VECTOR3, 2, data, sizes);
}

void CT_API SceneLoadCmdSetVector4(SceneLoadCommandBuffer cmd,
                                   const char* name,
                                   ctVec4 value) {
   ctAssert(cmd);
   void* data[] = {(void*)name, &value};
   size_t sizes[] = {strlen(name) + 1, sizeof(value)};
   cmd->AddCommand(HB_SCNLOADCMD_SET_VECTOR4, 2, data, sizes);
}

void CT_API SceneLoadCmdSetString(SceneLoadCommandBuffer cmd,
                                  const char* name,
                                  const char* value) {
   ctAssert(cmd);
   void* data[] = {(void*)name, (void*)value};
   size_t sizes[] = {strlen(name) + 1, strlen(value) + 1};
   cmd->AddCommand(HB_SCNLOADCMD_SET_STRING, 2, data, sizes);
}

void CT_API SceneLoadCmdLoadBarrier(SceneLoadCommandBuffer cmd) {
   ctAssert(cmd);
   cmd->AddCommand(HB_SCNLOADCMD_LOAD_BARRIER);
}

void CT_API SceneLoadCmdSpawnStageGLTF(SceneLoadCommandBuffer cmd,
                                       const char* fileName,
                                       const char* mapPathBase,
                                       ctVec3 pos,
                                       ctVec3 eulerXYZ) {
   ctAssert(cmd);
   void* data[] = {(void*)fileName, (void*)mapPathBase, &pos, &eulerXYZ};
   size_t sizes[] = {
     strlen(fileName) + 1, strlen(fileName) + 1, sizeof(pos), sizeof(eulerXYZ)};
   cmd->AddCommand(HB_SCNLOADCMD_SET_STRING, 2, data, sizes);
}

void CT_API SceneLoadCmdSpawnToy(SceneLoadCommandBuffer cmd,
                                 const char* prefabPath,
                                 const char* mapPath,
                                 ctVec3 pos,
                                 ctVec3 eulerXYZ,
                                 const char* spawnCommand) {
   ctAssert(cmd);
   void* data[] = {
     (void*)prefabPath, (void*)mapPath, &pos, &eulerXYZ, (void*)spawnCommand};
   size_t sizes[] = {strlen(prefabPath) + 1,
                     strlen(mapPath) + 1,
                     sizeof(pos),
                     sizeof(eulerXYZ),
                     strlen(spawnCommand) + 1};
   cmd->AddCommand(HB_SCNLOADCMD_SET_STRING, 2, data, sizes);
}

void CT_API SceneLoadCmdSpawnToyRelative(SceneLoadCommandBuffer cmd,
                                         const char* prefabPath,
                                         const char* mapPath,
                                         const char* referencePath,
                                         ctVec3 pos,
                                         ctVec3 eulerXYZ,
                                         const char* spawnCommand) {
   ctAssert(cmd);
   void* data[] = {(void*)prefabPath,
                   (void*)mapPath,
                   (void*)referencePath,
                   &pos,
                   &eulerXYZ,
                   (void*)spawnCommand};
   size_t sizes[] = {strlen(prefabPath) + 1,
                     strlen(mapPath) + 1,
                     strlen(referencePath) + 1,
                     sizeof(pos),
                     sizeof(eulerXYZ),
                     strlen(spawnCommand) + 1};
   cmd->AddCommand(HB_SCNLOADCMD_SET_STRING, 2, data, sizes);
}

void CT_API SceneLoadCmdPruneToy(SceneLoadCommandBuffer cmd, const char* path) {
   ctAssert(cmd);
   cmd->AddCommand(HB_SCNLOADCMD_DEBUG_LOG, (void*)path, strlen(path) + 1);
}

void CT_API SceneLoadCmdPruneTag(SceneLoadCommandBuffer cmd, const char* tag) {
   ctAssert(cmd);
   cmd->AddCommand(HB_SCNLOADCMD_DEBUG_LOG, (void*)tag, strlen(tag) + 1);
}

ctResults CT_API SceneLoadCommandBufferExecute(SceneLoadCommandBuffer buffer,
                                               class Scene* scene) {
   ctAssert(buffer);
   return CT_SUCCESS;
}

}

/* ---------------- Expose Scripting API ---------------- */

namespace ctScriptApi {
namespace Honeybell {
   void SceneLoadCommandBufferAbort(ctScriptTypedLightData buffer) {
      if (buffer.typeId != CT_SCRIPTOBTYPE_HONEYBELL_LOADCMDBUFFER) { return; }
      ctHoneybell::SceneLoadCommandBufferAbort(
        *(ctHoneybell::SceneLoadCommandBuffer*)buffer.ptr);
   }

   void SceneLoadCmdLog(ctScriptTypedLightData cmd, const char* text) {
      if (cmd.typeId != CT_SCRIPTOBTYPE_HONEYBELL_LOADCMDBUFFER) { return; }
      ctHoneybell::SceneLoadCmdLog(*(ctHoneybell::SceneLoadCommandBuffer*)cmd.ptr, text);
   }

   void SceneLoadCmdOpenSection(ctScriptTypedLightData cmd, const char* section) {
      if (cmd.typeId != CT_SCRIPTOBTYPE_HONEYBELL_LOADCMDBUFFER) { return; }
      ctHoneybell::SceneLoadCmdOpenSection(*(ctHoneybell::SceneLoadCommandBuffer*)cmd.ptr,
                                           section);
   }

   void SceneLoadCmdSetBool(ctScriptTypedLightData cmd, const char* name, bool value) {
      if (cmd.typeId != CT_SCRIPTOBTYPE_HONEYBELL_LOADCMDBUFFER) { return; }
      ctHoneybell::SceneLoadCmdSetBool(
        *(ctHoneybell::SceneLoadCommandBuffer*)cmd.ptr, name, value);
   }
   void
   SceneLoadCmdSetScalar(ctScriptTypedLightData cmd, const char* name, double value) {
      if (cmd.typeId != CT_SCRIPTOBTYPE_HONEYBELL_LOADCMDBUFFER) { return; }
      ctHoneybell::SceneLoadCmdSetScalar(
        *(ctHoneybell::SceneLoadCommandBuffer*)cmd.ptr, name, value);
   }
   void SceneLoadCmdSetVector2(ctScriptTypedLightData cmd,
                               const char* name,
                               float x,
                               float y) {
      if (cmd.typeId != CT_SCRIPTOBTYPE_HONEYBELL_LOADCMDBUFFER) { return; }
      ctHoneybell::SceneLoadCmdSetVector2(
        *(ctHoneybell::SceneLoadCommandBuffer*)cmd.ptr, name, ctVec2(x, y));
   }
   void SceneLoadCmdSetVector3(
     ctScriptTypedLightData cmd, const char* name, float x, float y, float z) {
      if (cmd.typeId != CT_SCRIPTOBTYPE_HONEYBELL_LOADCMDBUFFER) { return; }
      ctHoneybell::SceneLoadCmdSetVector3(
        *(ctHoneybell::SceneLoadCommandBuffer*)cmd.ptr, name, ctVec3(x, y, z));
   }
   void SceneLoadCmdSetVector4(
     ctScriptTypedLightData cmd, const char* name, float x, float y, float z, float w) {
      if (cmd.typeId != CT_SCRIPTOBTYPE_HONEYBELL_LOADCMDBUFFER) { return; }
      ctHoneybell::SceneLoadCmdSetVector4(
        *(ctHoneybell::SceneLoadCommandBuffer*)cmd.ptr, name, ctVec4(x, y, z, w));
   }
   void SceneLoadCmdSetString(ctScriptTypedLightData cmd,
                              const char* name,
                              const char* value) {
      if (cmd.typeId != CT_SCRIPTOBTYPE_HONEYBELL_LOADCMDBUFFER) { return; }
      ctHoneybell::SceneLoadCmdSetString(
        *(ctHoneybell::SceneLoadCommandBuffer*)cmd.ptr, name, value);
   }

   void SceneLoadCmdLoadBarrier(ctScriptTypedLightData cmd) {
      if (cmd.typeId != CT_SCRIPTOBTYPE_HONEYBELL_LOADCMDBUFFER) { return; }
      ctHoneybell::SceneLoadCmdLoadBarrier(
        *(ctHoneybell::SceneLoadCommandBuffer*)cmd.ptr);
   }
   void SceneLoadCmdSpawnStageGLTF(ctScriptTypedLightData cmd,
                                   const char* fileName,
                                   const char* mapPathBase,
                                   float px,
                                   float py,
                                   float pz,
                                   float ex,
                                   float ey,
                                   float ez) {
      if (cmd.typeId != CT_SCRIPTOBTYPE_HONEYBELL_LOADCMDBUFFER) { return; }
      ctHoneybell::SceneLoadCmdSpawnStageGLTF(
        *(ctHoneybell::SceneLoadCommandBuffer*)cmd.ptr,
        fileName,
        mapPathBase,
        ctVec3(px, py, pz),
        ctVec3(ex, ey, ez));
   }
   void SceneLoadCmdSpawnToy(ctScriptTypedLightData cmd,
                             const char* prefabPath,
                             const char* mapPath,
                             float px,
                             float py,
                             float pz,
                             float ex,
                             float ey,
                             float ez,
                             const char* spawnCommand) {
   }
   void SceneLoadCmdSpawnToyRelative(ctScriptTypedLightData cmd,
                                     const char* prefabPath,
                                     const char* mapPath,
                                     const char* referencePath,
                                     float px,
                                     float py,
                                     float pz,
                                     float ex,
                                     float ey,
                                     float ez,
                                     const char* spawnCommand) {
   }
   void SceneLoadCmdPruneToy(ctScriptTypedLightData cmd, const char* path) {
   }
   void SceneLoadCmdPruneTag(ctScriptTypedLightData cmd, const char* tag) {
   }
}
}