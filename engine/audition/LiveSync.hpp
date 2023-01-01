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
#include "core/ModuleBase.hpp"

/* Live sync provides a mechanism for editors to send props to engine for preview */

/* todo: redo and send full reconstruct packets (object spawners, material settings) */

enum ctAuditionLiveSyncCategory {
   CT_AUDITION_SYNC_CATEGORY_INTERNAL = 0,
   CT_AUDITION_SYNC_CATEGORY_CAMERA = 1,
   CT_AUDITION_SYNC_CATEGORY_SCENE = 2,
   CT_AUDITION_SYNC_CATEGORY_MATERIAL = 3,
   CT_AUDITION_SYNC_CATEGORY_COUNT
};

enum ctAuditionLiveSyncValueType {
   CT_AUDITION_SYNC_NULL = 0,
   CT_AUDITION_SYNC_NUMBER = 1,
   CT_AUDITION_SYNC_BOOL = 2,
   CT_AUDITION_SYNC_STRING = 3,
   CT_AUDITION_SYNC_VEC2 = 4,
   CT_AUDITION_SYNC_VEC3 = 5,
   CT_AUDITION_SYNC_VEC4 = 6,
   CT_AUDITION_SYNC_QUAT = 7,
   CT_AUDITION_SYNC_MAT4 = 8
};

class CT_API ctAuditionLiveSyncProp {
public:
   ctAuditionLiveSyncProp() {
      memset(this, 0, sizeof(*this));
   }
   ctAuditionLiveSyncProp(uint8_t category,
                          const char* asset,
                          const char* group,
                          const char* name) {
      p.propCategory = category;
      strncpy(p.asset, asset, 32);
      strncpy(p.group, group, 32);
      strncpy(p.propName, name, 30);
      p.valueType = CT_AUDITION_SYNC_NULL;
   }
   ctAuditionLiveSyncProp(uint8_t category,
                          const char* asset,
                          const char* group,
                          const char* name,
                          double value) {
      ctAuditionLiveSyncProp(category, asset, group, name);
      p.valueType = CT_AUDITION_SYNC_NUMBER;
      *(double*)p.storage = value;
   }
   ctAuditionLiveSyncProp(uint8_t category,
                          const char* asset,
                          const char* group,
                          const char* name,
                          bool value) {
      ctAuditionLiveSyncProp(category, asset, group, name);
      p.valueType = CT_AUDITION_SYNC_NUMBER;
      *(bool*)p.storage = value;
   }
   ctAuditionLiveSyncProp(uint8_t category,
                          const char* asset,
                          const char* group,
                          const char* name,
                          const char* value) {
      ctAuditionLiveSyncProp(category, asset, group, name);
      p.valueType = CT_AUDITION_SYNC_STRING;
      strncpy((char*)p.storage, value, 30);
   }
   ctAuditionLiveSyncProp(uint8_t category,
                          const char* asset,
                          const char* group,
                          const char* name,
                          ctVec2 value) {
      ctAuditionLiveSyncProp(category, asset, group, name);
      p.valueType = CT_AUDITION_SYNC_VEC2;
      *(ctVec2*)p.storage = value;
   }
   ctAuditionLiveSyncProp(uint8_t category,
                          const char* asset,
                          const char* group,
                          const char* name,
                          ctVec3 value) {
      ctAuditionLiveSyncProp(category, asset, group, name);
      p.valueType = CT_AUDITION_SYNC_VEC3;
      *(ctVec3*)p.storage = value;
   }
   ctAuditionLiveSyncProp(uint8_t category,
                          const char* asset,
                          const char* group,
                          const char* name,
                          ctVec4 value) {
      ctAuditionLiveSyncProp(category, asset, group, name);
      p.valueType = CT_AUDITION_SYNC_VEC4;
      *(ctVec4*)p.storage = value;
   }
   ctAuditionLiveSyncProp(uint8_t category,
                          const char* asset,
                          const char* group,
                          const char* name,
                          ctQuat value) {
      ctAuditionLiveSyncProp(category, asset, group, name);
      p.valueType = CT_AUDITION_SYNC_QUAT;
      *(ctQuat*)p.storage = value;
   }
   ctAuditionLiveSyncProp(uint8_t category,
                          const char* asset,
                          const char* group,
                          const char* name,
                          ctMat4 value) {
      ctAuditionLiveSyncProp(category, asset, group, name);
      p.valueType = CT_AUDITION_SYNC_MAT4;
      *(ctMat4*)p.storage = value;
   }

   friend class ctAuditionLiveSync;

   const char* GetAssetName() {
      return p.asset;
   }
   const char* GetGroupName() {
      return p.group;
   }
   const char* GetPropName() {
      return p.propName;
   }
   time_t GetTimestamp() {
      return timestamp;
   }
   ctAuditionLiveSyncValueType GetValueType() {
      return (ctAuditionLiveSyncValueType)p.valueType;
   }

   inline double GetNumber(double defaultVal = 0.0) {
      return p.valueType == CT_AUDITION_SYNC_NUMBER ? *(double*)p.storage : defaultVal;
   }
   inline bool GetBool(bool defaultVal = false) {
      return p.valueType == CT_AUDITION_SYNC_BOOL ? *(bool*)p.storage : defaultVal;
   }
   inline const char* GetString(const char* defaultVal = NULL) {
      return p.valueType == CT_AUDITION_SYNC_STRING ? (const char*)p.storage : defaultVal;
   }
   inline ctVec2 GetVec2(ctVec2 defaultValue = ctVec2()) {
      return p.valueType == CT_AUDITION_SYNC_VEC2 ? *(ctVec2*)p.storage : defaultValue;
   }
   inline ctVec3 GetVec3(ctVec3 defaultValue = ctVec3()) {
      return p.valueType == CT_AUDITION_SYNC_VEC3 ? *(ctVec3*)p.storage : defaultValue;
   }
   inline ctVec4 GetVec4(ctVec4 defaultValue = ctVec4()) {
      return p.valueType == CT_AUDITION_SYNC_VEC4 ? *(ctVec4*)p.storage : defaultValue;
   }
   inline ctQuat GetQuat(ctQuat defaultValue = ctQuat()) {
      return p.valueType == CT_AUDITION_SYNC_QUAT ? *(ctQuat*)p.storage : defaultValue;
   }
   inline ctMat4 GetMat4(ctMat4 defaultValue = ctMat4()) {
      return p.valueType == CT_AUDITION_SYNC_MAT4 ? *(ctMat4*)p.storage : defaultValue;
   }

protected:
private:
   uint64_t timestamp;

   /* data changes will require changes to clients */
   struct Packet {
      uint8_t propCategory; /* eg: camera, scene, material */
      uint8_t valueType;    /* ctAuditionLiveSyncValueType */
      char propName[30];    /* name of the property */
      char asset[32];       /* asset nickname */
      char group[32];       /* group name (eg: objects) */
      uint8_t storage[64];  /* storage */
   } p;
};

typedef void (*ctAuditionLiveSyncNotifyCallback)(ctAuditionLiveSyncProp prop,
                                                 void* userData);

class CT_API ctAuditionLiveSync : public ctModuleBase {
public:
   ctResults Startup() final;
   ctResults Shutdown() final;
   const char* GetModuleName() final;
   virtual void DebugUI(bool useGizmos);

   void StartServer();
   void StopServer();
   bool isRunning();

   inline void SetProp(ctAuditionLiveSyncProp& prop) {
      ctMutexLock(lock);
      locked.properties[prop.p.propCategory].Append(prop);
      ctMutexUnlock(lock);
   }
   inline void BroadcastProp(ctAuditionLiveSyncProp& prop) {
      ctMutexLock(lock);
      locked.outgoing.Append(prop);
      ctMutexUnlock(lock);
   }

   inline void ClearProps(uint8_t category) {
      ctMutexLock(lock);
      locked.properties[category].Clear();
      ctMutexUnlock(lock);
   }

   void Dispatch(ctAuditionLiveSyncCategory category, bool onlyChanged = true);

private:
   static int ServerMain(void*);
   int ServerLoop();

   int32_t stagedPort = 4892;

   ctMutex lock;
   struct {
      bool shouldRun;
      int32_t port;

      ctDynamicArray<ctAuditionLiveSyncProp> outgoing;
      uint64_t lastDispatchTime[CT_AUDITION_SYNC_CATEGORY_COUNT];
      ctDynamicArray<ctAuditionLiveSyncProp> properties[CT_AUDITION_SYNC_CATEGORY_COUNT];

      struct CallbackData {
         ctAuditionLiveSyncNotifyCallback fpCallback;
         void* pData;
      };
      CallbackData callbacks[CT_AUDITION_SYNC_CATEGORY_COUNT];
   } locked;
   ctThread serverThread;
};