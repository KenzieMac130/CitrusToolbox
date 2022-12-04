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
#include "gpu/Struct.h"

struct ctGPUStructAssembler {
   uint16_t size; /* unaligned by default */
   uint16_t largestBaseAlignment;
   uint16_t sizes[31];
   uint16_t offsets[31]; /* UINT16_MAX is terminator */
};