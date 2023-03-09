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

/* bring-your-own-system entity-component kernel */

typedef uint32_t ctSceneEntity;
typedef struct ctSceneComponentQueryT* ctSceneComponentQuery;
typedef struct ctSceneComponentFetcherT* ctSceneComponentFetcher;
typedef struct ctSceneContextT* ctSceneContext;

typedef void (*ctSceneDestructorFunc)(void* pComponentData, void* pUserData);
typedef bool (*ctSceneCustomFilterFunc)(const void* pComponentData, const void* pUserData);

typedef struct {
   size_t entityCountReserve;
   size_t componentMemPoolReserve;
} ctSceneContextCreateDesc;

typedef struct {
   const char* name;
   size_t componentSize;
   ctSceneDestructorFunc fpDestructor;
   void* pUserData;
} ctSceneComponentTypeDesc;

typedef struct {
   const char* name;
   ctSceneEntity entity;
   void* pContents;
} ctSceneComponentCreateDesc;

#define CT_SCNQUERY_READ(_COMPONENT) "r\""_COMPONENT "\""
#define CT_SCNQUERY_WRITE(_COMPONENT) "w\""_COMPONENT "\""
#define CT_SCNQUERY_READWRITE(_COMPONENT) "rw\""_COMPONENT "\""

#define CT_SCNQUERY_READ_OPT(_COMPONENT) "*r\""_COMPONENT "\""
#define CT_SCNQUERY_WRITE_OPT(_COMPONENT) "*w\""_COMPONENT "\""
#define CT_SCNQUERY_READWRITE_OPT(_COMPONENT) "*rw\""_COMPONENT "\""

#define CT_SCNQUERY_READ_FILTER(_COMPONENT) "fr\""_COMPONENT "\""
#define CT_SCNQUERY_WRITE_FILTER(_COMPONENT) "fw\""_COMPONENT "\""
#define CT_SCNQUERY_READWRITE_FILTER(_COMPONENT) "frw\""_COMPONENT "\""

#define CT_SCNQUERY_READ_OPT_FILTER(_COMPONENT) "f*r\""_COMPONENT "\""
#define CT_SCNQUERY_WRITE_OPT_FILTER(_COMPONENT) "f*w\""_COMPONENT "\""
#define CT_SCNQUERY_READWRITE_OPT_FILTER(_COMPONENT) "f*rw\""_COMPONENT "\""

#define CT_SCNQUERY_READ_FETCH(_COMPONENT) "mr\""_COMPONENT "\""
#define CT_SCNQUERY_WRITE_FETCH(_COMPONENT) "mw\""_COMPONENT "\""
#define CT_SCNQUERY_READWRITE_FETCH(_COMPONENT) "mrw\""_COMPONENT "\""

#define CT_SCNQUERY_INCLUDE(_COMPONENT) "#\""_COMPONENT "\""
#define CT_SCNQUERY_EXCLUDE(_COMPONENT) "!\""_COMPONENT "\""

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
   "...": name of component */
   const char* expression;
   size_t customFilterCount;
   ctSceneCustomFilterFunc* fpCustomFilters;
   void** ppUserData;
} ctSceneComponentQueryCreateDesc;

typedef struct {
   void* pComponents[8];
   void* _state1[8];
   size_t _state2[8];
   void* _state3;
} ctSceneComponentIterator;

// clang-format off
enum ctResults ctSceneContextCreate(ctSceneContext* pContext, ctSceneContextCreateDesc* pDesc);
enum ctResults ctSceneContextDestroy(ctSceneContext context);

enum ctResults ctSceneComponentTypeRegister(ctSceneContext context, ctSceneComponentTypeDesc* pDesc);

enum ctResults ctSceneEntityCreate(ctSceneContext context, ctSceneEntity* pEntity);
enum ctResults ctSceneEntityDestroy(ctSceneContext context, ctSceneEntity entity);

enum ctResults ctSceneComponentCreate(ctSceneContext context, ctSceneComponentCreateDesc* pDesc);
enum ctResults ctSceneComponentDestroy(ctSceneContext context, ctSceneEntity entity, const char* name);

/* for cached lookups */
enum ctResults ctSceneComponentQueryCreate(ctSceneContext context, ctSceneComponentQuery* pQuery, ctSceneComponentQueryCreateDesc* pDesc);
enum ctResults ctSceneComponentQueryDestroy(ctSceneContext context, ctSceneComponentQuery query);

enum ctResults ctSceneComponentQueryBeginUse(ctSceneContext context,
                                              ctSceneComponentQuery query);
size_t ctSceneComponentQueryLoopCount(ctSceneComponentQuery query);
void** ctSceneComponentQueryPointers(ctSceneComponentQuery query);
enum ctResults ctSceneComponentQueryEndUse(ctSceneContext context, ctSceneComponentQuery query);

/* for non-cached lookups */
enum ctResults ctSceneComponentIteratorBeginUse(ctSceneContext context, ctSceneComponentQueryCreateDesc* pDesc, ctSceneComponentIterator* pIter);
bool ctSceneComponentIteratorNextEntity(ctSceneComponentIterator* pIter);
enum ctResults ctSceneComponentIteratorEndUse(ctSceneContext context, ctSceneComponentIterator iter);

/* for single component lookups */
enum ctResults ctSceneComponentGetFetcher(ctSceneContext context, ctSceneComponentFetcher* pFetcher, const char* componentName);
void* ctSceneComponentFetch(ctSceneComponentFetcher fetcher, ctSceneEntity entity);
// clang-format on