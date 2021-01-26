/* Temporary test */

#include "utilities/Reflect.gen.hpp"

class mine {};

#pragma ct class {json = true, imgui = false}
class doot : mine {
public:
#pragma ct var { serialize = true }
   int integer;

#pragma ct var { serialize = true }
   bool boolean = false;

#pragma ct var { serialize = true }
   bool arr[32];

/*Not valid*/
#pragma ct var { serialize = true }
   bool* ptr;
};