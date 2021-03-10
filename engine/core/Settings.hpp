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
#include "ModuleBase.hpp"
#include "FileSystem.hpp"

/*Todo: This immediately, needs to be able to register and load console values*/

class ctSettings : ctModuleBase {
public:
   ctSettings(const ctStringUtf8& fileName, ctFileSystem* pFileSystem);

   ctResults Startup() final;
   ctResults Shutdown() final;

private:
   struct _setting {
      uint8_t type;
      uint32_t idx;
   };
   ctHashTable<_setting> _commandAssociations;
   ctStaticArray<int32_t, CT_MAX_SETTINGS_INTS> _ints;
   ctStaticArray<float, CT_MAX_SETTINGS_FLOATS> _floats;
   ctStaticArray<ctStringUtf8, CT_MAX_SETTINGS_FLOATS> _strings;
};