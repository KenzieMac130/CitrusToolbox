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
#include "utilities/SpacialQuery.hpp"

namespace ctHoneybell {

struct SignalDesc {
   int32_t priority;
   const char* path;
   int flags;
   float value;
   ctTransform targetTransform;
   ctHandle originToy;
   float deltaTime;
};

struct SignalContext {
   inline SignalContext() {
      memset(this, 0, sizeof(*this));
      boundsSphere = ctBoundSphere();
      targetTransform = ctTransform();
   }
   uint32_t flags;              /* Additional flags to pass */
   int32_t priority;            /* Priority that this signal took */
   uint64_t typeHash;           /* Signal type hash (horner) */
   ctBoundSphere boundsSphere;  /* Transform of the signal origin */
   ctTransform targetTransform; /* Transform of the signal target */
   float value;                 /* Scalar value of the signal */
   ctHandle originToy;          /* The toy that broadcasted the signal (if any) */
   float deltaTime;             /* Delta time when signal was processed */
};

struct SignalDispatchEntry {
   int32_t priority;
   class ToyBase* pToy;
   uint32_t signalIdx;
};

class CT_API SignalManager {
public:
   SignalManager();
   ~SignalManager();
   friend class Scene;

   void BroadcastGlobalSignal(SignalDesc& desc);
   void BroadcastSpacialSignal(SignalDesc& desc, ctBoundSphere sphere);
   void BroadcastTargetedSignal(SignalDesc& desc, ctHandle toyHandle);

protected:
   void AddToy(class ToyBase* toy);
   void RemoveToy(class ToyBase* toy);
   void AddSpacialToy(class ToyBase* toy);
   void RetrackSpacialToy(class ToyBase* toy, ctVec3 oldPosition);
   void RemoveSpacialToy(class ToyBase* toy);

   void DispatchSignals();
   void Reset();

private:
   ctMutex signalLock;

   bool cacheOutOfDate = false;
   void RebuildCache();

   void BuildGlobalSignals();
   void BuildTargetedSignals();
   void BuildSpacialSignals();
   void ExecuteBuiltSignals();

   struct {
      ctDynamicArray<SignalContext> signals;
   } globalSignals;
   struct {
      ctDynamicArray<ctBoundSphere> spheres;
      ctDynamicArray<SignalContext> signals;
   } spacialSignals;
   struct {
      ctDynamicArray<ctHandle> toyHandles;
      ctDynamicArray<SignalContext> signals;
   } targetedSignals;

   ctDynamicArray<SignalContext> outSignals;
   ctDynamicArray<SignalDispatchEntry> dispatchEntries;

   /* Track toys which can recieve signals */
   ctDynamicArray<ctHandle> signalToyHanldes;
   ctDynamicArray<ToyBase*> signalToys;
   ctBloomFilter<ctHandle, 1024, 4> signalToysBloom;
   ctHashTable<ToyBase*, ctHandle> signalToysByHandle;
   ctSpacialQuery signalToysSpacial;
};
}