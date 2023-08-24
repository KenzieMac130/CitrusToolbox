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
#include "JoltContext.hpp"
#include "Jolt/Physics/Collision/Shape/BoxShape.h"
#include "Jolt/Physics/Collision/Shape/SphereShape.h"
#include "Jolt/Physics/Collision/Shape/CapsuleShape.h"
#include "Jolt/Physics/Collision/Shape/ConvexHullShape.h"
#include "Jolt/Physics/Collision/Shape/MeshShape.h"
#include "Jolt/Physics/Collision/Shape/StaticCompoundShape.h"
#include "Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h"
#include "Jolt/Physics/Collision/Shape/ScaledShape.h"
#include "Jolt/Physics/Collision/Shape/OffsetCenterOfMassShape.h"

enum CreateShapeFromCitrusLayer {
   CT_JOLT_SHAPE_LAYER_BASE,
   CT_JOLT_SHAPE_LAYER_COMPOUND,
   CT_JOLT_SHAPE_LAYER_CENTER_OF_MASS,
   CT_JOLT_SHAPE_LAYER_SCALE,
   CT_JOLT_SHAPE_LAYER_TRANSFORM
};

JPH::Shape::ShapeResult
CreateShapeFromCitrus(const ctPhysicsEngine& ctx,
                      ctPhysicsShapeSettings& desc,
                      CreateShapeFromCitrusLayer layer = CT_JOLT_SHAPE_LAYER_TRANSFORM);