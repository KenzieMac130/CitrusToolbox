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

#include "Common.h"

#include "cglm/cglm.h"

/* Hard-coded coordinate spaces (based on glTF) */
// clang-format off
#define CT_UP        {  0.0f,  1.0f,  0.0f  } 
#define CT_DOWN      {  0.0f, -1.0f,  0.0f  }
#define CT_FORWARD   {  0.0f,  0.0f,  1.0f  }
#define CT_BACK      {  0.0f,  0.0f, -1.0f  } 
#define CT_RIGHT     {  1.0f,  0.0f,  0.0f  }
#define CT_LEFT      { -1.0f,  0.0f,  0.0f  }
// clang-format on

struct CT_API ctVec2 {
   union {
      struct {
         float x;
         float y;
      };
      float data[2];
   };
};

struct CT_API ctVec3 {
   union {
      struct {
         float x;
         float y;
         float z;
      };
      float data[3];
   };
};

struct CT_API ctVec4 {
   union {
      struct {
         float x;
         float y;
         float z;
         float w;
      };
      float data[4];
   };
};

typedef ctVec4 ctQuat;