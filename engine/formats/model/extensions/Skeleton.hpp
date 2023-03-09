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
#include "../Model.hpp"

struct ctModelSkeletonBoneTransform {
   float translation[3];
   float rotation[4];
   float scale[3];
};

struct ctModelSkeletonBoneGraph {
   int32_t parent;
   int32_t firstChild;
   int32_t nextSibling;
};

struct ctModelSkeletonBoneName {
   uint32_t hash;
   int32_t stringLookup;
};

struct ctModelSkeleton {
uint32_t boneCount;
ctModelSkeletonBoneTransform* transformArray;
ctModelSkeletonBoneGraph* graphArray;
ctModelSkeletonBoneName* nameArray;
};

CT_API ctModelSkeleton* ctModelGetSkeleton(ctModel model);