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

/* Temporary test */

#include "Common.h"

#include "utilities/Reflect.gen.hpp"

class mine {};

#pragma ct class {json = true, imgui = false }
class doot : mine {
public:
#pragma ct var { serialize = true }
   int integer;

#pragma ct var { serialize = true }
   bool boolean = false;

#pragma ct var { serialize = true }
   bool arr[32];

#pragma ct var {serialize = true }
   ctDynamicArray<int> dArr;

#pragma ct var {serialize = true }
   ctStaticArray<int, 8> sArr;

/*Not valid*/
#pragma ct var { serialize = false }
   bool* ptr;
};