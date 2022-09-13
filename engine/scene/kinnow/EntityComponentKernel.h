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

/* Internally locking bring-your-own-system entity-component kernel */

typedef uint32_t ctKinnowEntity;
typedef struct ctKinnowComponentQueryT* ctKinnowComponentQuery;
typedef struct ctKinnowComponentFetcherT* ctKinnowComponentFetcher;
typedef struct ctKinnowWorldT* ctKinnowWorld;

typedef void (*ctKinnowDestructorFunc)(void* pComponentData, void* pUserData);
typedef bool (*ctKinnowCustomFilterFunc)(void* pComponentData, void* pUserData);

typedef struct {
   size_t entityCountReserve;
   size_t componentMemPoolReserve;
} ctKinnowWorldCreateDesc;

typedef struct {
   const char* name;
   size_t componentSize;
   ctKinnowDestructorFunc fpDestructor;
   void* pUserData;
} ctKinnowComponentTypeDesc;

typedef struct {
   const char* name;
   ctKinnowEntity entity;
   void* pContents;
} ctKinnowComponentCreateDesc;

typedef struct {
   /* expression:
   *: optional modifier (needs a null check)
   r: fetch component for read
   w: fetch component for write
   rw: fetch component for read and write
   f: uses a filter function to approve initial check (add to anything)
   m: random access manual fetch (requires r/w/rw, don't add to args)
   #: tag only (don't add to args)
   !: tag only exclude (don't add to args)
   "...": name of component
   i: arg is entity id */
   const char* expression;
   size_t customFilterCount;
   ctKinnowCustomFilterFunc* pfpCustomFilters;
   void** ppUserData;
} ctKinnowComponentQueryCreateDesc;

typedef struct {
   void* pComponents[8];
   void* _state1[8];
   size_t _state2[8];
   void* _state3;
} ctKinnowComponentIterator;

// clang-format off
enum ctResults ctKinnowWorldCreate(ctKinnowWorld* pWorld, ctKinnowWorldCreateDesc* pDesc);
enum ctResults ctKinnowWorldDestroy(ctKinnowWorld world);

enum ctResults ctKinnowComponentTypeRegister(ctKinnowWorld world, ctKinnowComponentTypeDesc* pDesc);

enum ctResults ctKinnowEntityCreate(ctKinnowWorld world, ctKinnowEntity* pEntity);
enum ctResults ctKinnowEntityDestroy(ctKinnowWorld world, ctKinnowEntity entity);

enum ctResults ctKinnowComponentCreate(ctKinnowWorld world, ctKinnowComponentCreateDesc* pDesc);
enum ctResults ctKinnowComponentDestroy(ctKinnowWorld world, ctKinnowEntity entity, const char* name);

enum ctResults ctKinnowComponentQueryCreate(ctKinnowWorld world, ctKinnowComponentQuery* pQuery, ctKinnowComponentQueryCreateDesc* pDesc);
enum ctResults ctKinnowComponentQueryDestroy(ctKinnowWorld world, ctKinnowComponentQuery query);

enum ctResults ctKinnowComponentQueryBeginUse(ctKinnowWorld world,
                                              ctKinnowComponentQuery query);
size_t ctKinnowComponentQueryLoopCount(ctKinnowComponentQuery query);
void** ctKinnowComponentQueryPointers(ctKinnowComponentQuery query);
enum ctResults ctKinnowComponentQueryEndUse(ctKinnowWorld world, ctKinnowComponentQuery query);

enum ctResults ctKinnowComponentIteratorBeginUse(ctKinnowWorld world, ctKinnowComponentQueryCreateDesc* pDesc, ctKinnowComponentIterator* pIter);
bool ctKinnowComponentIteratorNextEntity(ctKinnowComponentIterator* pIter);
enum ctResults ctKinnowComponentIteratorEndUse(ctKinnowWorld world, ctKinnowComponentIterator iter);

void* ctKinnowComponentGetFetcher(ctKinnowWorld world, const char* componentName);
void* ctKinnowComponentFetch(ctKinnowComponentFetcher comType, ctKinnowEntity entity);
// clang-format on