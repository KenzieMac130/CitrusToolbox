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
#include "EntityComponentKernel.h"

/* ----------------------- Concrete types ----------------------- */

struct ctKinnowComponentQueryT {
   // ctKinnowComponentQueryT(const char* format);
   // void QueryComponents(struct ctKinnowWorld& world);

   // todo: baked query
   /* flattened continuous array of pointers for each component */
   ctDynamicArray<void*> absoluteAddressForComponents;
};

struct ctKinnowComponentFetcherT {
   struct ctKinnowComponentTypeT* pManager;
};

struct ctKinnowComponentTypeT {
   ctKinnowComponentTypeT(ctKinnowComponentTypeDesc& desc);
   ~ctKinnowComponentTypeT();
   void ExpandEntityCount(size_t newCount);

   ctStringUtf8 name;
   uint32_t componentHash; /* used for serialization sanity checks */
   size_t componentSize;   /* can be 0 for flags */
   /* offset from start of component in blob */
   // todo: bloom filter
   ctDynamicArray<int32_t>
     componentOffsets; /* offsets into entity blobs indexed by handle */
   // ctDynamicArray<ctKinnowComponentQuery*> /* dependent queries on add/remove/resize */
};

struct ctKinnowWorldT {
   ctKinnowWorldT(ctKinnowWorldCreateDesc& desc);
   ~ctKinnowWorldT();

   size_t maxEntities;

   void RegisterComponentType(ctKinnowComponentTypeDesc& desc);
   ctHashTable<ctKinnowComponentTypeT*, uint32_t> componentTypesByName;
   ctDynamicArray<ctKinnowComponentTypeT*> componentTypes; /* component types */

   ctHandleManager entityManager;
   ctDynamicArray<size_t> entityBlobStarts; /* indexed by handle */

   ctResults AddComponent(ctKinnowComponentCreateDesc& desc);

   /* component memory allocator */
   ctResults AllocateComponent();
   size_t nextFreeChunk; /* next 64 bytes */
   ctDynamicArray<size_t> chunksFree;
   ctDynamicArray<uint8_t> componentMemoryBlob;

   // ctDynamicArray<ctKinnowComponentQuery*> /* dependent queries on add/remove/resize */
};

/* ----------------------- API ----------------------- */

ctResults ctKinnowWorldCreate(ctKinnowWorld* pWorld, ctKinnowWorldCreateDesc* pDesc) {
   *pWorld = new ctKinnowWorldT(*pDesc);
   return CT_SUCCESS;
}

ctResults ctKinnowWorldDestroy(ctKinnowWorld world) {
   delete world;
   return CT_SUCCESS;
}

ctResults ctKinnowComponentTypeRegister(ctKinnowWorld world,
                                        ctKinnowComponentTypeDesc* pDesc) {
   world->RegisterComponentType(*pDesc);
   return CT_SUCCESS;
}

ctResults ctKinnowEntityCreate(ctKinnowWorld world, ctKinnowEntity* pEntity) {
   *pEntity = world->entityManager.GetNewHandle();
   return CT_SUCCESS;
}

ctResults ctKinnowEntityDestroy(ctKinnowWorld world, ctKinnowEntity entity) {
   world->entityManager.FreeHandle(entity);
   /* todo: release all other component types */
   return CT_SUCCESS;
}

ctResults ctKinnowComponentCreate(ctKinnowWorld world,
                                  ctKinnowComponentCreateDesc* pDesc) {
   /* find component type by name */
   return ctResults();
}

/* ----------------------- Internals ----------------------- */

ctKinnowWorldT::ctKinnowWorldT(ctKinnowWorldCreateDesc& desc) {
   maxEntities = desc.entityCountReserve;
}

ctKinnowWorldT::~ctKinnowWorldT() {
   for (size_t i = 0; i < componentTypes.Count(); i++) {
      delete componentTypes[i];
   }
}

void ctKinnowWorldT::RegisterComponentType(ctKinnowComponentTypeDesc& desc) {
   ctKinnowComponentTypeT* componentType = new ctKinnowComponentTypeT(desc);
   componentType->ExpandEntityCount(maxEntities);
   componentTypes.Append(componentType);
   componentTypesByName.Insert(ctXXHash32(desc.name), componentType);
}

ctResults ctKinnowWorldT::AddComponent(ctKinnowComponentCreateDesc& desc) {
   ctKinnowComponentTypeT** ppType = componentTypesByName.FindPtr(ctXXHash32(desc.name));
   if (!ppType) { return CT_FAILURE_NOT_FOUND; }
   ctKinnowComponentTypeT& type = **ppType;

   // todo call component allocator
   entityBlobStarts[desc.entity];      /* hmm... */
   type.componentOffsets[desc.entity]; /* hmm... */
   return CT_SUCCESS;
}

ctKinnowComponentTypeT::ctKinnowComponentTypeT(ctKinnowComponentTypeDesc& desc) {
   name = desc.name;
   componentHash = ctXXHash32(desc.name);
   componentSize = desc.componentSize;
}

ctKinnowComponentTypeT::~ctKinnowComponentTypeT() {
}

void ctKinnowComponentTypeT::ExpandEntityCount(size_t newCount) {
   size_t initialCount = componentOffsets.Count();
   componentOffsets.Resize(newCount);
   /* invalidate all unused */
   for (size_t i = initialCount; i < componentOffsets.Count(); i++) {
      componentOffsets[i] = INT_MAX;
   }
}
