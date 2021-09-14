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

#include "Signal.hpp"
#include "Toy.hpp"

ctHoneybell::SignalManager::SignalManager() {
   ZoneScoped;
   globalSignals.signals.Reserve(512);

   spacialSignals.spheres.Reserve(512);
   spacialSignals.signals.Reserve(512);

   targetedSignals.toyHandles.Reserve(512);
   targetedSignals.signals.Reserve(512);

   signalToyHanldes.Reserve(1024);
   signalToys.Reserve(1024);
   signalToysByHandle.Reserve(1031);
   signalToysSpacial.Reserve(1031);

   outSignals.Reserve(2048);
   dispatchEntries.Reserve(2048);

   signalLock = ctMutexCreate();
}

ctHoneybell::SignalManager::~SignalManager() {
   ZoneScoped;
   ctMutexDestroy(signalLock);
}

void ctHoneybell::SignalManager::BroadcastGlobalSignal(SignalDesc& desc) {
   ZoneScoped;
   ctMutexLock(signalLock);
   SignalContext ctx = SignalContext();
   ctx.priority = desc.priority;
   ctx.typeHash = ctHornerHash(desc.path);
   ctx.flags = desc.flags;
   ctx.value = desc.value;
   ctx.targetTransform = desc.targetTransform;
   ctx.originToy = desc.originToy;
   ctx.deltaTime = desc.deltaTime;
   globalSignals.signals.Append(ctx);
   ctMutexUnlock(signalLock);
}

void ctHoneybell::SignalManager::BroadcastSpacialSignal(SignalDesc& desc,
                                                        ctBoundSphere sphere) {
   ZoneScoped;
   ctMutexLock(signalLock);
   SignalContext ctx = SignalContext();
   ctx.priority = desc.priority;
   ctx.typeHash = ctHornerHash(desc.path);
   ctx.flags = desc.flags;
   ctx.value = desc.value;
   ctx.targetTransform = desc.targetTransform;
   ctx.originToy = desc.originToy;
   ctx.boundsSphere = sphere;
   ctx.deltaTime = desc.deltaTime;
   spacialSignals.spheres.Append(sphere);
   spacialSignals.signals.Append(ctx);
   ctMutexUnlock(signalLock);
}

void ctHoneybell::SignalManager::BroadcastTargetedSignal(SignalDesc& desc,
                                                         ctHandle toyHandle) {
   ZoneScoped;
   ctMutexLock(signalLock);
   SignalContext ctx = SignalContext();
   ctx.priority = desc.priority;
   ctx.typeHash = ctHornerHash(desc.path);
   ctx.flags = desc.flags;
   ctx.value = desc.value;
   ctx.targetTransform = desc.targetTransform;
   ctx.originToy = desc.originToy;
   ctx.deltaTime = desc.deltaTime;
   targetedSignals.toyHandles.Append(toyHandle);
   targetedSignals.signals.Append(ctx);
   ctMutexUnlock(signalLock);
}

void ctHoneybell::SignalManager::AddToy(ToyBase* toy) {
   ZoneScoped;
   signalToys.Append(toy);
   const ctHandle hndl = toy->GetIdentifier();
   signalToysBloom.Insert(hndl);
   signalToysByHandle.Insert(hndl, toy);
   signalToyHanldes.Append(hndl);
}

void ctHoneybell::SignalManager::RemoveToy(ToyBase* toy) {
   ZoneScoped;
   ctHandle hndl = toy->GetIdentifier();
   if (!signalToysBloom.MightExist(hndl)) { return; }
   signalToysByHandle.Remove(hndl);
   signalToys.Remove(toy);
   signalToyHanldes.Remove(hndl);
   cacheOutOfDate = true;
}

void ctHoneybell::SignalManager::AddSpacialToy(ToyBase* toy) {
   ZoneScoped;
   signalToysSpacial.Add(toy->GetIdentifier(),
                         ctSpacialCellKey(toy->GetWorldTransform().position / 4));
}

void ctHoneybell::SignalManager::RetrackSpacialToy(ToyBase* toy, ctVec3 oldPosition) {
   ZoneScoped;
   signalToysSpacial.Remove(toy->GetIdentifier(), ctSpacialCellKey(oldPosition / 4));
   AddSpacialToy(toy);
}

void ctHoneybell::SignalManager::RemoveSpacialToy(ToyBase* toy) {
   ZoneScoped;
   signalToysSpacial.Remove(toy->GetIdentifier(),
                            ctSpacialCellKey(toy->GetWorldTransform().position / 4));
}

void ctHoneybell::SignalManager::RebuildCache() {
   ZoneScoped;
   signalToysBloom.Reset();
   for (size_t i = 0; i < signalToyHanldes.Count(); i++) {
      signalToysBloom.Insert(signalToyHanldes[i]);
   }
}

void ctHoneybell::SignalManager::DispatchSignals() {
   ZoneScoped;
   ctAssert(spacialSignals.signals.Count() == spacialSignals.spheres.Count());
   ctAssert(targetedSignals.signals.Count() == targetedSignals.toyHandles.Count());
   if (cacheOutOfDate) { RebuildCache(); }
   BuildGlobalSignals();
   BuildTargetedSignals();
   BuildSpacialSignals();
   ExecuteBuiltSignals();
   Reset();
}

void ctHoneybell::SignalManager::Reset() {
   globalSignals.signals.Clear();
   targetedSignals.signals.Clear();
   targetedSignals.toyHandles.Clear();
   spacialSignals.signals.Clear();
   spacialSignals.spheres.Clear();
   outSignals.Clear();
   dispatchEntries.Clear();
}

void ctHoneybell::SignalManager::BuildGlobalSignals() {
   ZoneScoped;
   for (size_t i = 0; i < globalSignals.signals.Count(); i++) {
      for (int j = 0; j < signalToys.Count(); j++) {
         dispatchEntries.Append({globalSignals.signals[i].priority,
                                 signalToys[j],
                                 (uint32_t)outSignals.Count()});
         outSignals.Append(globalSignals.signals[i]);
      }
   }
}

void ctHoneybell::SignalManager::BuildTargetedSignals() {
   ZoneScoped;
   for (size_t i = 0; i < targetedSignals.signals.Count(); i++) {
      ToyBase** ppToy = signalToysByHandle.FindPtr(targetedSignals.toyHandles[i]);
      if (ppToy) {
         dispatchEntries.Append(
           {targetedSignals.signals[i].priority, *ppToy, (uint32_t)outSignals.Count()});
         outSignals.Append(targetedSignals.signals[i]);
      }
   }
}

void ctHoneybell::SignalManager::BuildSpacialSignals() {
   ZoneScoped;
   for (size_t i = 0; i < spacialSignals.signals.Count(); i++) {
      const ctBoundBox bounds = spacialSignals.spheres[i].ToBox();
      for (float x = bounds.min.x; x <= bounds.max.x; x += 4.0f) {
         for (float y = bounds.min.y; y <= bounds.max.y; y += 4.0f) {
            for (float z = bounds.min.z; z <= bounds.max.z; z += 4.0f) {
               const ctSpacialCellKey key = ctSpacialCellKey(ctVec3(x, y, z) / 4);
               uint32_t bucketCount = signalToysSpacial.GetBucketCount(key);
               for (uint32_t b = 0; b < bucketCount; b++) {
                  const ctSpacialCellBucket* pBucket =
                    signalToysSpacial.GetBucket(key, b);
                  for (int t = 0; t < CT_MAX_SPACIAL_QUERY_ENTRIES_PER_CELL; t++) {
                     if (pBucket->entries[t]) {
                        ToyBase** ppToy = signalToysByHandle.FindPtr(pBucket->entries[t]);
                        if (ppToy) {
                           dispatchEntries.Append({spacialSignals.signals[i].priority,
                                                   *ppToy,
                                                   (uint32_t)outSignals.Count()});
                           outSignals.Append(spacialSignals.signals[i]);
                        }
                     }
                  }
               }
            }
         }
      }
   }
}

int SignalDispatchSortCompareFunc(const ctHoneybell::SignalDispatchEntry* A,
                                  const ctHoneybell::SignalDispatchEntry* B) {
   ctAssert(A);
   ctAssert(B);
   return A->priority - B->priority;
}

void ctHoneybell::SignalManager::ExecuteBuiltSignals() {
   dispatchEntries.QSort(SignalDispatchSortCompareFunc);
   for (size_t i = 0; i < dispatchEntries.Count(); i++) {
      ctAssert(dispatchEntries[i].pToy);
      dispatchEntries[i].pToy->_CallSignal(outSignals[dispatchEntries[i].signalIdx]);
   }
}
