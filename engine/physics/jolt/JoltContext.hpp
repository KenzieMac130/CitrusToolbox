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

/* jolt has a coding style which clashes with the rest of citrus... */

/* jolt flags are manually specified here */
#if !(CITRUS_IS_PRODUCTION && !CITRUS_IS_DEBUG)
#define JPH_DEBUG_RENDERER
#define JPH_PROFILE_ENABLED
#endif

#include "Jolt/Jolt.h"
#include "../Physics.hpp"

#include "Jolt/Core/TempAllocator.h"
#include "Jolt/Core/JobSystemWithBarrier.h"
#include "Jolt/Core/FixedSizeFreeList.h"
#include "Jolt/Core/Factory.h"
#include "Jolt/RegisterTypes.h"
#include "Jolt/Physics/PhysicsSettings.h"
#include "Jolt/Physics/PhysicsSystem.h"

#include "core/JobSystem.hpp"
#include "../shared/Surface.hpp"

#define ctVec3ToJolt(v)   JPH::Vec3(v.x, v.y, v.z)
#define ctVec4ToJolt(v)   JPH::Vec4(v.x, v.y, v.z, v.w)
#define ctQuatToJolt(v)   JPH::Quat(v.x, v.y, v.z, v.w)
#define ctVec3ToJoltF3(v) JPH::Float3(v.x, v.y, v.z)
#define ctVec4ToJoltColor(v)                                                             \
   JPH::Color((JPH::uint8)(v.r * 255),                                                   \
              (JPH::uint8)(v.g * 255),                                                   \
              (JPH::uint8)(v.b * 255),                                                   \
              (JPH::uint8)(v.a * 255))

#define ctVec3FromJolt(v)   ctVec3(v.GetX(), v.GetY(), v.GetZ())
#define ctVec4FromJolt(v)   ctVec4(v.GetX(), v.GetY(), v.GetZ(), v.GetW())
#define ctQuatFromJolt(v)   ctQuat(v.GetX(), v.GetY(), v.GetZ(), v.GetW())
#define ctVec3FromJoltF3(v) ctVec3(v.x, v.y, v.z)
#define ctVec4FromJoltColor(v)                                                           \
   ctVec4((float)v.r / 255, (float)v.g / 255, (float)v.b / 255, (float)v.a / 255)

inline JPH::EMotionType ctPhysicsMotionTypeToJolt(ctPhysicsMotionType type) {
   switch (type) {
      case CT_PHYSICS_STATIC: return JPH::EMotionType::Static;
      case CT_PHYSICS_KINEMATIC: return JPH::EMotionType::Kinematic;
      case CT_PHYSICS_DYNAMIC: return JPH::EMotionType::Dynamic;
      default: return JPH::EMotionType::Static;
   }
}

inline JPH::EAllowedDOFs ctPhysicsDOFToJolt(ctPhysicsDOF type) {
   JPH::EAllowedDOFs result = JPH::EAllowedDOFs::None;
   if (ctCFlagCheck(type, CT_PHYSICS_DOF_TRANSLATION_X)) {
      result |= JPH::EAllowedDOFs::TranslationX;
   }
   if (ctCFlagCheck(type, CT_PHYSICS_DOF_TRANSLATION_Y)) {
      result |= JPH::EAllowedDOFs::TranslationY;
   }
   if (ctCFlagCheck(type, CT_PHYSICS_DOF_TRANSLATION_Z)) {
      result |= JPH::EAllowedDOFs::TranslationZ;
   }
   if (ctCFlagCheck(type, CT_PHYSICS_DOF_ROTATION_X)) {
      result |= JPH::EAllowedDOFs::RotationX;
   }
   if (ctCFlagCheck(type, CT_PHYSICS_DOF_ROTATION_X)) {
      result |= JPH::EAllowedDOFs::RotationY;
   }
   if (ctCFlagCheck(type, CT_PHYSICS_DOF_ROTATION_X)) {
      result |= JPH::EAllowedDOFs::RotationZ;
   }
   return result;
}

class CitrusJoltObjectLayerPairFilter : public JPH::ObjectLayerPairFilter {
public:
   inline CitrusJoltObjectLayerPairFilter(ctPhysicsCollisionMatrix matrix) {
      colmatrix = matrix;
   }
   virtual bool ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2);

private:
   ctPhysicsCollisionMatrix colmatrix;
};

class CitrusJoltObjectVsBroadPhaseLayerFilter :
    public JPH::ObjectVsBroadPhaseLayerFilter {
public:
   virtual bool ShouldCollide(JPH::ObjectLayer inLayer1,
                              JPH::BroadPhaseLayer inLayer2) const override;
};

enum CitrusJoltBroadPhaseLayers {
   CT_JOLT_BP_STATIC,
   CT_JOLT_BP_DYNAMIC,
   CT_JOLT_BP_COUNT
};

class CitrusJoltBroadphaseLayerInterface final : public JPH::BroadPhaseLayerInterface {
public:
   inline CitrusJoltBroadphaseLayerInterface() {
   }
   virtual JPH::uint GetNumBroadPhaseLayers() const override;
   virtual JPH::BroadPhaseLayer
   GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override;
#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
   virtual const char*
   GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override;
#endif
private:
};

class CitrusJoltJobSystem : public JPH::JobSystemWithBarrier {
public:
   CitrusJoltJobSystem() {
      JobSystemWithBarrier::Init(2048);
      jobs.Init(2048, 2048);
   }
   virtual int GetMaxConcurrency() const;
   virtual JobHandle CreateJob(const char* name,
                               JPH::ColorArg unused1,
                               const JobFunction& inJobFunction,
                               JPH::uint32 dependencies = 0);
   virtual void FreeJob(Job* inJob);
   virtual void QueueJob(Job* inJob);
   virtual void QueueJobs(Job** inJobs, unsigned int inNumJobs);

private:
   static void ExecuteJob(void* data);
   JPH::FixedSizeFreeList<Job> jobs;
};

class CitrusJoltPhysicsMaterial : public JPH::PhysicsMaterial {
public:
   CitrusJoltPhysicsMaterial(ctPhysicsSurfaceTypeT* pSurf) {
      pSurface = pSurf;
      debugColor =
        ctVec4ToJoltColor(ctRandomGenerator(ctXXHash32(pSurface->GetName())).GetColor());
   }

   virtual const char* GetDebugName() const {
      return pSurface->GetName();
   }

   virtual JPH::Color GetDebugColor() const {
      return debugColor;
   }

   ctPhysicsSurfaceTypeT* pSurface;
   JPH::Color debugColor;
};

class CitrusJoltStreamOut : public JPH::StreamOut {
public:
   inline CitrusJoltStreamOut(ctDynamicArray<uint8_t>* dest) {
      pOutArray = dest;
   }
   virtual ~CitrusJoltStreamOut() = default;

   virtual void WriteBytes(const void* inData, size_t inNumBytes);
   virtual bool IsFailed() const;

   inline int64_t Tell() {
       return (int64_t)pOutArray->Count();
   }

   inline void* Ptr(size_t offset) {
       return &(*pOutArray)[offset];
   }

private:
   ctDynamicArray<uint8_t>* pOutArray;
};

class CitrusJoltStreamIn : public JPH::StreamIn {
public:
   CitrusJoltStreamIn(uint8_t* dest, size_t size);

   virtual void ReadBytes(void* outData, size_t inNumBytes);
   inline void Seek(int64_t offset, ctFileSeekMode mode = CT_FILE_SEEK_SET) {
      switch (mode) {
         case CT_FILE_SEEK_SET: seek = offset; break;
         case CT_FILE_SEEK_CUR: seek = offset + seek; break;
         case CT_FILE_SEEK_END: seek = offset + maxSize; break;
         default: break;
      }
   }
   inline int64_t Tell() {
      return (int64_t)seek;
   }
   inline void* CurrPtr() {
      return ptr + seek;
   }
   virtual bool IsEOF() const;
   virtual bool IsFailed() const;

private:
   uint8_t* ptr;
   size_t maxSize;
   size_t seek;
};

struct ctPhysicsEngineT {
   JPH::TempAllocatorImpl* pTempAllocator;
   CitrusJoltJobSystem* pJobSystem;
   CitrusJoltObjectLayerPairFilter* pObjectLayerFilter;
   CitrusJoltObjectVsBroadPhaseLayerFilter* pBroadphaseFilter;
   CitrusJoltBroadphaseLayerInterface* pBroadphaseLayerInterface;
   JPH::PhysicsSystem physics;

   bool pollStatistics;
   bool allowRenderDebug;
   JPH::PhysicsSystem::BodyStats lastStatistics;

#ifdef JPH_DEBUG_RENDERER
   class CitrusJoltDebugRenderer* pDebugRenderer;
#endif

   /* no lock needed, runtime surface type creation not allowed */
   ctPhysicsSurfaceTypeFactory* pSurfaceFactory;
   ctHashTable<CitrusJoltPhysicsMaterial*, uint32_t> physicsMaterials;

   inline JPH::PhysicsMaterial* GetMaterialForSurfaceType(uint32_t hash) const {
      CitrusJoltPhysicsMaterial** ppMaterial = physicsMaterials.FindPtr(hash);
      if (!ppMaterial) {
         ppMaterial = physicsMaterials.FindPtr(ctXXHash32("default"));
         if (!ppMaterial) { return NULL; }
         return (JPH::PhysicsMaterial*)*ppMaterial;
      }
      return (JPH::PhysicsMaterial*)*ppMaterial;
   }
};