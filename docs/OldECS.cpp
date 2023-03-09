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

struct ctSceneComponentQueryT {
   // ctSceneComponentQueryT(const char* format);
   // void QueryComponents(struct ctSceneContext& context);

   // todo: baked query
   /* flattened continuous array of pointers for each component */
   ctDynamicArray<void*> absoluteAddressForComponents;
};

struct ctSceneComponentFetcherT {
   struct ctSceneComponentTypeT* pManager;
};

struct ctSceneComponentTypeT {
   ctSceneComponentTypeT(ctSceneComponentTypeDesc& desc);
   ~ctSceneComponentTypeT();
   void ExpandEntityCount(size_t newCount);

   ctStringUtf8 name;
   uint32_t componentHash; /* used for serialization sanity checks */
   size_t componentSize;   /* can be 0 for flags */
   /* offset from start of component in blob */
   // todo: bloom filter
   ctDynamicArray<int32_t>
     componentOffsets; /* offsets into entity blobs indexed by handle */
   // ctDynamicArray<ctSceneComponentQuery*> /* dependent queries on add/remove/resize */
};

struct ctSceneContextT {
   ctSceneContextT(ctSceneContextCreateDesc& desc);
   ~ctSceneContextT();

   size_t maxEntities;

   void RegisterComponentType(ctSceneComponentTypeDesc& desc);
   ctHashTable<ctSceneComponentTypeT*, uint32_t> componentTypesByName;
   ctDynamicArray<ctSceneComponentTypeT*> componentTypes; /* component types */

   ctHandleManager entityManager;
   ctDynamicArray<size_t> entityBlobStarts; /* indexed by handle */

   ctResults AddComponent(ctSceneComponentCreateDesc& desc);

   /* component memory allocator */
   ctResults AllocateComponent();
   size_t nextFreeChunk; /* next 64 bytes */
   ctDynamicArray<size_t> chunksFree;
   ctDynamicArray<uint8_t> componentMemoryBlob;

   // ctDynamicArray<ctSceneComponentQuery*> /* dependent queries on add/remove/resize */
};

/* ----------------------- API ----------------------- */

ctResults ctSceneContextCreate(ctSceneContext* pContext, ctSceneContextCreateDesc* pDesc) {
   *pContext = new ctSceneContextT(*pDesc);
   return CT_SUCCESS;
}

ctResults ctSceneContextDestroy(ctSceneContext context) {
   delete context;
   return CT_SUCCESS;
}

ctResults ctSceneComponentTypeRegister(ctSceneContext context,
                                        ctSceneComponentTypeDesc* pDesc) {
   context->RegisterComponentType(*pDesc);
   return CT_SUCCESS;
}

ctResults ctSceneEntityCreate(ctSceneContext context, ctSceneEntity* pEntity) {
   *pEntity = context->entityManager.GetNewHandle();
   return CT_SUCCESS;
}

ctResults ctSceneEntityDestroy(ctSceneContext context, ctSceneEntity entity) {
   context->entityManager.FreeHandle(entity);
   /* todo: release all other component types */
   return CT_SUCCESS;
}

ctResults ctSceneComponentCreate(ctSceneContext context,
                                  ctSceneComponentCreateDesc* pDesc) {
   /* find component type by name */
   return ctResults();
}

/* ----------------------- Internals ----------------------- */

ctSceneContextT::ctSceneContextT(ctSceneContextCreateDesc& desc) {
   maxEntities = desc.entityCountReserve;
}

ctSceneContextT::~ctSceneContextT() {
   for (size_t i = 0; i < componentTypes.Count(); i++) {
      delete componentTypes[i];
   }
}

void ctSceneContextT::RegisterComponentType(ctSceneComponentTypeDesc& desc) {
   ctSceneComponentTypeT* componentType = new ctSceneComponentTypeT(desc);
   componentType->ExpandEntityCount(maxEntities);
   componentTypes.Append(componentType);
   componentTypesByName.Insert(ctXXHash32(desc.name), componentType);
}

ctResults ctSceneContextT::AddComponent(ctSceneComponentCreateDesc& desc) {
   ctSceneComponentTypeT** ppType = componentTypesByName.FindPtr(ctXXHash32(desc.name));
   if (!ppType) { return CT_FAILURE_NOT_FOUND; }
   ctSceneComponentTypeT& type = **ppType;

   // todo call component allocator
   entityBlobStarts[desc.entity];      /* hmm... */
   type.componentOffsets[desc.entity]; /* hmm... */
   return CT_SUCCESS;
}

ctSceneComponentTypeT::ctSceneComponentTypeT(ctSceneComponentTypeDesc& desc) {
   name = desc.name;
   componentHash = ctXXHash32(desc.name);
   componentSize = desc.componentSize;
}

ctSceneComponentTypeT::~ctSceneComponentTypeT() {
}

void ctSceneComponentTypeT::ExpandEntityCount(size_t newCount) {
   size_t initialCount = componentOffsets.Count();
   componentOffsets.Resize(newCount);
   /* invalidate all unused */
   for (size_t i = initialCount; i < componentOffsets.Count(); i++) {
      componentOffsets[i] = INT_MAX;
   }
}
