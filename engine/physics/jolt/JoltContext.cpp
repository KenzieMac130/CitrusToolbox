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

#include "JoltContext.hpp"
#include "JoltDebugRender.hpp"

/* ------------------ Global Callbacks ------------------ */
void CitrusJoltTrace(const char* format, ...) {
   va_list va;
   va_start(va, format);
   _ctDebugLogCallLoggerVa(0, format, va);
   va_end(va);
}

void* CitrusJoltAllocate(size_t size) {
   return ctMalloc(size);
}

void CitrusJoltFree(void* ptr) {
   ctFree(ptr);
}

void* CitrusJoltAlignedAllocate(size_t size, size_t alignment) {
   return ctAlignedMalloc(size, alignment);
}

void CitrusJoltAlignedFree(void* ptr) {
   return ctAlignedFree(ptr);
}

/* ------------------- Job System Wrapper ------------------- */
int CitrusJoltJobSystem::GetMaxConcurrency() const {
   return (int)ctGetJobSystem()->GetThreadCount();
}

/* similar to JobSystemThreadPool */
CitrusJoltJobSystem::JobHandle
CitrusJoltJobSystem::CreateJob(const char* inJobName,
                               JPH::ColorArg inColor,
                               const JobFunction& inJobFunction,
                               JPH::uint32 inNumDependencies) {
   JPH::uint32 index;
   for (;;) {
      index =
        jobs.ConstructObject(inJobName, inColor, this, inJobFunction, inNumDependencies);
      if (index != jobs.cInvalidObjectIndex) break;
      ctDebugError("No jolt jobs were availible...");
      ctWait(1);
   }
   Job* job = &jobs.Get(index);
   JobHandle handle(job);
   if (inNumDependencies == 0) { QueueJob(job); }
   return handle;
}

void CitrusJoltJobSystem::FreeJob(Job* inJob) {
   jobs.DestructObject(inJob);
}

void CitrusJoltJobSystem::ExecuteJob(void* data) {
   Job* job = (Job*)data;
   job->Execute();
   job->Release();
}

void CitrusJoltJobSystem::QueueJob(Job* inJob) {
   inJob->AddRef();
   ctGetJobSystem()->PushJob(ExecuteJob, (void*)inJob);
}

void CitrusJoltJobSystem::QueueJobs(Job** inJobs, unsigned int inNumJobs) {
   /* technically inefficient as we have to acquire the spinlock every iteration */
   for (unsigned int i = 0; i < inNumJobs; i++) {
      QueueJob(inJobs[i]);
   }
}

/* ------------------- Layers and Broadphase ------------------- */

bool CitrusJoltObjectLayerPairFilter::ShouldCollide(JPH::ObjectLayer inObject1,
                                                    JPH::ObjectLayer inObject2) {
   return colmatrix.DoesCollide((ctPhysicsLayer)inObject1, (ctPhysicsLayer)inObject2);
}

bool CitrusJoltObjectVsBroadPhaseLayerFilter::ShouldCollide(
  JPH::ObjectLayer objLayer, JPH::BroadPhaseLayer bp2) const {
   /* only the first physlayer is static which doesn't collide with itself */
   if (objLayer == 0) {
      return (CitrusJoltBroadPhaseLayers)(JPH::BroadPhaseLayer::Type)bp2 ==
             CT_JOLT_BP_DYNAMIC;
   } else {
      return true;
   }
}

JPH::uint CitrusJoltBroadphaseLayerInterface::GetNumBroadPhaseLayers() const {
   return CT_JOLT_BP_COUNT;
}

JPH::BroadPhaseLayer
CitrusJoltBroadphaseLayerInterface::GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const {
   return (JPH::BroadPhaseLayer)((ctPhysicsLayer)inLayer == CT_PHYSICS_LAYER_STATIC
                                   ? CT_JOLT_BP_STATIC
                                   : CT_JOLT_BP_DYNAMIC);
}

const char* CitrusJoltBroadphaseLayerInterface::GetBroadPhaseLayerName(
  JPH::BroadPhaseLayer inLayer) const {
   switch ((CitrusJoltBroadPhaseLayers)(JPH::BroadPhaseLayer::Type)inLayer) {
      case CT_JOLT_BP_STATIC: return "STATIC";
      case CT_JOLT_BP_DYNAMIC: return "DYNAMIC";
      default: return "INVALID";
   }
}

/* ------------------- Statrup ------------------- */
ctResults ctPhysicsEngineStartup(ctPhysicsEngine& ctx, ctPhysicsEngineDesc& desc) {
   ctDebugLog("Starting Jolt...");

   /* Setup allocator and trace */
   JPH::Allocate = CitrusJoltAllocate;
   JPH::AlignedAllocate = CitrusJoltAlignedAllocate;
   JPH::Free = CitrusJoltFree;
   JPH::AlignedFree = CitrusJoltAlignedFree;
   JPH::Trace = CitrusJoltTrace;

   /* Setup factory and types */
   JPH::Factory::sInstance = new JPH::Factory();
   JPH::RegisterTypes();

   /* Create Context */
   ctx = new ctPhysicsEngineT();

   /* Setup temporary memory allocator for physics updates */
   ctx->pTempAllocator =
     new JPH::TempAllocatorImpl((JPH::uint32)desc.scratchAllocationSize);

   /* setup interfaces */
   ctx->pJobSystem = new CitrusJoltJobSystem();
   ctx->pBroadphaseLayerInterface = new CitrusJoltBroadphaseLayerInterface();
   ctx->pObjectLayerFilter = new CitrusJoltObjectLayerPairFilter(desc.collisionMatrix);
   ctx->pBroadphaseFilter = new CitrusJoltObjectVsBroadPhaseLayerFilter();

   /* Initialize the physics system */
   ctx->physics.Init((JPH::uint)desc.maxBodies,
                     0,
                     (JPH::uint)desc.maxBroadphasePairs,
                     (JPH::uint)desc.maxContactConstraints,
                     *ctx->pBroadphaseLayerInterface,
                     *ctx->pBroadphaseFilter,
                     *ctx->pObjectLayerFilter);

   /* materials from surface types */
   if (desc.surfaceTypeJSON) {
      ctJSONReader json = ctJSONReader();
      json.BuildJsonForPtr(desc.surfaceTypeJSON, strlen(desc.surfaceTypeJSON));
      ctJSONReadEntry root;
      if (json.GetRootEntry(root) == CT_SUCCESS) {
         ctx->pSurfaceFactory = new ctPhysicsSurfaceTypeFactory(root);
         auto surfaceTypes = ctx->pSurfaceFactory->GetSurfaceMap();
         for (auto it = surfaceTypes.GetIterator(); it; it++) {
            ctx->physicsMaterials.Insert(ctXXHash32(it.Value()->GetName()),
                                         new CitrusJoltPhysicsMaterial(it.Value()));
         }
      }
   }

   /* debug ui */
   ctx->pollStatistics = true;
   ctx->allowRenderDebug = true;
   ctx->lastStatistics = JPH::PhysicsSystem::BodyStats();

   /* shape caching */
   ctSpinLockInit(ctx->bakeShapeLock);

   /* debug rendering */
#ifdef JPH_DEBUG_RENDERER
   ctx->bodyDraw = JPH::BodyManager::DrawSettings();
   ctx->drawConstraints = true;
   ctx->pDebugRenderer = new CitrusJoltDebugRenderer();
   JPH::DebugRenderer::sInstance = ctx->pDebugRenderer;
#endif

   return CT_SUCCESS;
}

ctResults ctPhysicsEngineShutdown(ctPhysicsEngine ctx) {
   /* we assumed the api user waited for all jobs to finish */
   ctx->bakeShapeCache.Clear();
   JPH::UnregisterTypes();
   delete JPH::Factory::sInstance;
   JPH::Factory::sInstance = NULL;
   delete ctx->pBroadphaseFilter;
   delete ctx->pObjectLayerFilter;
   delete ctx->pBroadphaseLayerInterface;
   delete ctx->pTempAllocator;
   delete ctx->pJobSystem;
#ifdef JPH_DEBUG_RENDERER
   delete ctx->pDebugRenderer;
#endif
   delete ctx;
   return CT_SUCCESS;
}

ctResults ctPhysicsEngineUpdate(ctPhysicsEngine ctx, float deltaTime, int32_t steps) {
   ctx->physics.Update(deltaTime, steps, ctx->pTempAllocator, ctx->pJobSystem);
   return CT_SUCCESS;
}

#include "imgui/imgui.h"

ctResults ctPhysicsEngineExecDebugDraw(ctPhysicsEngine ctx) {
   if (!ctx->allowRenderDebug) { return CT_SUCCESS; }
#ifdef JPH_DEBUG_RENDERER
   ctx->physics.DrawBodies(ctx->bodyDraw, ctx->pDebugRenderer);
   if (ctx->drawConstraints) { ctx->physics.DrawConstraints(ctx->pDebugRenderer); }
   if (ctx->drawConstraintLimits) {
      ctx->physics.DrawConstraintLimits(ctx->pDebugRenderer);
   }
   if (ctx->drawConstraintReferenceFrame) {
      ctx->physics.DrawConstraintReferenceFrame(ctx->pDebugRenderer);
   }
   ctx->pDebugRenderer->Render();
#endif
   return CT_SUCCESS;
}

ctResults ctPhysicsEngineExecDebugUI(ctPhysicsEngine ctx) {
   ImGui::Checkbox("Debug Draw", &ctx->allowRenderDebug);
   ImGui::Checkbox("Get Statistics", &ctx->pollStatistics);
   if (ctx->pollStatistics) { ctx->lastStatistics = ctx->physics.GetBodyStats(); }
   JPH::PhysicsSystem::BodyStats& stats = ctx->lastStatistics;
   ImGui::Text("Bodies: %u/%u\nStatic: %u\nDynamic: %u-%u\nKinematic: %u-%u\nSoft: %u-%u",
               stats.mNumBodies,
               stats.mMaxBodies,
               stats.mNumBodiesStatic,
               stats.mNumActiveBodiesDynamic,
               stats.mNumBodiesDynamic,
               stats.mNumActiveBodiesKinematic,
               stats.mNumBodiesKinematic,
               stats.mNumActiveSoftBodies,
               stats.mNumSoftBodies);
   if (ImGui::CollapsingHeader("Draw Options")) {
      ImGui::Checkbox("Shape", &ctx->bodyDraw.mDrawShape);
      ImGui::Checkbox("Shape Wireframe", &ctx->bodyDraw.mDrawShapeWireframe);
      const char* shapeColorOptions[] {
        "Instance", "Shape Type", "Motion Type", "Sleep", "Island", "Material"};
      ImGui::Combo("Shape Color",
                   (int*)&ctx->bodyDraw.mDrawShapeColor,
                   shapeColorOptions,
                   ctCStaticArrayLen(shapeColorOptions),
                   -1);

      // ImGui::Checkbox("Support Function", &ctx->bodyDraw.mDrawGetSupportFunction);
      ImGui::Checkbox("Support Direction", &ctx->bodyDraw.mDrawSupportDirection);
      // ImGui::Checkbox("Supporting Face", &ctx->bodyDraw.mDrawGetSupportingFace);

      ImGui::Checkbox("Bounding Box", &ctx->bodyDraw.mDrawBoundingBox);
      ImGui::Checkbox("Center of Mass Transform",
                      &ctx->bodyDraw.mDrawCenterOfMassTransform);
      ImGui::Checkbox("World Transform", &ctx->bodyDraw.mDrawWorldTransform);
      ImGui::Checkbox("Velocity", &ctx->bodyDraw.mDrawVelocity);
      ImGui::Checkbox("Mass and Interia", &ctx->bodyDraw.mDrawMassAndInertia);
      // ImGui::Checkbox("Sleep Stats", &ctx->bodyDraw.mDrawSleepStats);
      ImGui::Checkbox("Constraints", &ctx->drawConstraints);
      ImGui::Checkbox("Constraint Limits", &ctx->drawConstraintLimits);
      ImGui::Checkbox("Constraint Reference Frame", &ctx->drawConstraintReferenceFrame);
   }
   return CT_SUCCESS;
}

/* ----------------- Stream ----------------- */

void CitrusJoltStreamOut::WriteBytes(const void* inData, size_t inNumBytes) {
   pOutArray->Append((uint8_t*)inData, inNumBytes);
}

bool CitrusJoltStreamOut::IsFailed() const {
   return false;
}

CitrusJoltStreamIn::CitrusJoltStreamIn(uint8_t* dest, size_t size) {
   seek = 0;
   ptr = dest;
   maxSize = size;
}

void CitrusJoltStreamIn::ReadBytes(void* outData, size_t inNumBytes) {
   memcpy(outData, ptr + seek, inNumBytes);
   seek += inNumBytes;
}

bool CitrusJoltStreamIn::IsEOF() const {
   return seek == maxSize;
}

bool CitrusJoltStreamIn::IsFailed() const {
   return false;
}
