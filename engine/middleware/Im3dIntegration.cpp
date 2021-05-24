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

#include "Im3dIntegration.hpp"

ctResults ctIm3dIntegration::Startup() {
   return CT_SUCCESS;
}

ctResults ctIm3dIntegration::Shutdown() {
   return CT_SUCCESS;
}

ctResults ctIm3dIntegration::NextFrame() {
   Im3d::AppData& appData = Im3d::GetAppData();
   /* Todo: Feed me seymour */
   Im3d::NewFrame();
   return CT_SUCCESS;
}
