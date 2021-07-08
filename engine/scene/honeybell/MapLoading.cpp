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

#include "MapLoading.hpp"

struct CT_API ctMapLoadCommandHeader {
   ctMapLoadCmdType type;
   uint32_t nextCmdBytePos;
};

class ctMapLoadCommandBufferI {
public:
   inline ctMapLoadCommandBufferI() {
      bytes.Clear();
      canWrite = true;
      recording = false;
   }
   inline ctMapLoadCommandBufferI(const void* data, size_t size) {
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

   inline size_t CalcNextOffset(size_t size) {
      return bytes.Count() + sizeof(ctMapLoadCommandHeader) + size;
   }

   inline void AddCommand(ctMapLoadCmdType type) {
      ctMapLoadCommandHeader header = {type, CalcNextOffset(0)};
      AddData(&header, sizeof(header));
   }
   inline void AddCommand(ctMapLoadCmdType type, void* pData, size_t size) {
      ctMapLoadCommandHeader header = {type, CalcNextOffset(size)};
      AddData(&header, sizeof(header));
      AddData(pData, size);
   }
   inline void
   AddCommand(ctMapLoadCmdType type, size_t dataCount, void** ppData, size_t* pSizes) {
      size_t total = 0;
      for (size_t i = 0; i < dataCount; i++) {
         total += pSizes[i];
      }
      ctMapLoadCommandHeader header = {type, CalcNextOffset(total)};
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

ctResults CT_API ctMapLoadCommandBufferCreate(ctMapLoadCommandBuffer* pBuffer) {
   *pBuffer = new ctMapLoadCommandBufferI();
   return CT_SUCCESS;
}

ctResults CT_API ctMapLoadCommandBufferCreateFromBinary(ctMapLoadCommandBuffer* pBuffer,
                                                        const void* data,
                                                        size_t size) {
   *pBuffer = new ctMapLoadCommandBufferI(data, size);
   return CT_SUCCESS;
}

void CT_API ctMapLoadCommandBufferDestroy(ctMapLoadCommandBuffer buffer) {
   delete buffer;
}

ctResults CT_API ctMapLoadCommandBufferBegin(ctMapLoadCommandBuffer buffer) {
   ctAssert(buffer);
   return buffer->Begin();
}

ctResults CT_API ctMapLoadCommandBufferEnd(ctMapLoadCommandBuffer buffer) {
   ctAssert(buffer);
   return buffer->End();
}

void CT_API ctMapLoadCommandBufferAbort(ctMapLoadCommandBuffer buffer) {
   ctAssert(buffer);
   buffer->Abort();
}

void CT_API ctMapLoadCmdLog(ctMapLoadCommandBuffer cmd, const char* text) {
   ctAssert(cmd);
   cmd->AddCommand(CT_MAPLOADCMD_DEBUG_LOG, (void*)text, strlen(text) + 1);
}

void CT_API ctMapLoadCmdOpenSection(ctMapLoadCommandBuffer cmd, const char* section) {
   ctAssert(cmd);
   cmd->AddCommand(CT_MAPLOADCMD_OPEN_SECTION, (void*)section, strlen(section) + 1);
}

void CT_API ctMapLoadCmdSetBool(ctMapLoadCommandBuffer cmd,
                                const char* name,
                                bool value) {
   void* data[] = {(void*)name, &value};
   size_t sizes[] = {strlen(name) + 1, sizeof(value)};
   cmd->AddCommand(CT_MAPLOADCMD_SET_BOOL, 2, data, sizes);
   ctAssert(cmd);
}

void CT_API ctMapLoadCmdSetScalar(ctMapLoadCommandBuffer cmd,
                                  const char* name,
                                  double value) {
   ctAssert(cmd);
   void* data[] = {(void*)name, &value};
   size_t sizes[] = {strlen(name) + 1, sizeof(value)};
   cmd->AddCommand(CT_MAPLOADCMD_SET_SCALAR, 2, data, sizes);
}

void CT_API ctMapLoadCmdSetVector2(ctMapLoadCommandBuffer cmd,
                                   const char* name,
                                   ctVec2 value) {
   ctAssert(cmd);
   void* data[] = {(void*)name, &value};
   size_t sizes[] = {strlen(name) + 1, sizeof(value)};
   cmd->AddCommand(CT_MAPLOADCMD_SET_VECTOR2, 2, data, sizes);
}

void CT_API ctMapLoadCmdSetVector3(ctMapLoadCommandBuffer cmd,
                                   const char* name,
                                   ctVec3 value) {
   ctAssert(cmd);
   void* data[] = {(void*)name, &value};
   size_t sizes[] = {strlen(name) + 1, sizeof(value)};
   cmd->AddCommand(CT_MAPLOADCMD_SET_VECTOR3, 2, data, sizes);
}

void CT_API ctMapLoadCmdSetVector4(ctMapLoadCommandBuffer cmd,
                                   const char* name,
                                   ctVec4 value) {
   ctAssert(cmd);
   void* data[] = {(void*)name, &value};
   size_t sizes[] = {strlen(name) + 1, sizeof(value)};
   cmd->AddCommand(CT_MAPLOADCMD_SET_VECTOR4, 2, data, sizes);
}

void CT_API ctMapLoadCmdSetString(ctMapLoadCommandBuffer cmd,
                                  const char* name,
                                  const char* value) {
   ctAssert(cmd);
   void* data[] = {(void*)name, (void*)value};
   size_t sizes[] = {strlen(name) + 1, strlen(value) + 1};
   cmd->AddCommand(CT_MAPLOADCMD_SET_STRING, 2, data, sizes);
}

void CT_API ctMapLoadCmdLoadBarrier(ctMapLoadCommandBuffer cmd) {
   ctAssert(cmd);
   cmd->AddCommand(CT_MAPLOADCMD_LOAD_BARRIER);
}

void CT_API ctMapLoadCmdSpawnStageGLTF(ctMapLoadCommandBuffer cmd,
                                       const char* fileName,
                                       const char* mapPathBase,
                                       ctVec3 pos,
                                       ctVec3 eulerXYZ) {
   ctAssert(cmd);
   void* data[] = {(void*)fileName, (void*)mapPathBase, &pos, &eulerXYZ};
   size_t sizes[] = {
     strlen(fileName) + 1, strlen(fileName) + 1, sizeof(pos), sizeof(eulerXYZ)};
   cmd->AddCommand(CT_MAPLOADCMD_SET_STRING, 2, data, sizes);
}

void CT_API ctMapLoadCmdSpawnToy(ctMapLoadCommandBuffer cmd,
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
   cmd->AddCommand(CT_MAPLOADCMD_SET_STRING, 2, data, sizes);
}

void CT_API ctMapLoadCmdSpawnToyRelative(ctMapLoadCommandBuffer cmd,
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
   cmd->AddCommand(CT_MAPLOADCMD_SET_STRING, 2, data, sizes);
}

void CT_API ctMapLoadCmdPruneToy(ctMapLoadCommandBuffer cmd, const char* path) {
   ctAssert(cmd);
   cmd->AddCommand(CT_MAPLOADCMD_DEBUG_LOG, (void*)path, strlen(path) + 1);
}

void CT_API ctMapLoadCmdPruneTag(ctMapLoadCommandBuffer cmd, const char* tag) {
   ctAssert(cmd);
   cmd->AddCommand(CT_MAPLOADCMD_DEBUG_LOG, (void*)tag, strlen(tag) + 1);
}

ctResults CT_API ctMapLoadCommandBufferExecute(ctMapLoadCommandBuffer buffer,
                                               void* scene) {
   ctAssert(buffer);
   return CT_SUCCESS;
}
