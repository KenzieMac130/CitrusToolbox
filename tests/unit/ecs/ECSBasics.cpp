/*
   Copyright 2022 MacKenzie Strand

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

#include "scene/kinnow/EntityComponentKernel.h"

#define TEST_NO_MAIN
#include "acutest/acutest.h"

ctKinnowWorld world;

void kinnow_boilerplate() {
   ZoneScoped;
   ctKinnowWorldCreateDesc desc = {};
   desc.componentMemPoolReserve = 1024;
   desc.entityCountReserve = 4096;
   ctKinnowWorldCreate(&world, &desc);
}

void kinnow_shutdown() {
   ZoneScoped;
   ctKinnowWorldDestroy(world);
}

void kinnow_test_routine() {
   ZoneScoped;
   kinnow_boilerplate();
   kinnow_shutdown();
}

void kinnow_test_lots_of_component_types() {
   ZoneScoped;
   kinnow_boilerplate();
   for (int i = 0; i < 512; i++) {
      char name[32];
      snprintf(name, 32, "stresscomponent %i", i);
      ctKinnowComponentTypeDesc componentDesc = {};
      componentDesc.componentSize = 0;
      componentDesc.name = name;
      ctKinnowComponentTypeRegister(world, &componentDesc);
   }
   kinnow_shutdown();
}

void kinnow_add_entities() {
   ZoneScoped;
   kinnow_boilerplate();
   for (int i = 0; i < 4096; i++) {
      ctKinnowEntity ent;
      ctKinnowEntityCreate(world, &ent);
   }
   kinnow_shutdown();
}

void kinnow_add_components() {
   ZoneScoped;
   kinnow_boilerplate();

   ctKinnowComponentTypeDesc positionComponentDesc = {};
   positionComponentDesc.componentSize = sizeof(ctVec3);
   positionComponentDesc.name = "position";
   ctKinnowComponentTypeRegister(world, &positionComponentDesc);

   ctKinnowEntity ent;
   ctKinnowEntityCreate(world, &ent);

   ctKinnowComponentCreateDesc desc;
   desc.entity = ent;
   desc.name = "position";
   desc.pContents = NULL;
   ctKinnowComponentCreate(world, &desc);

   kinnow_shutdown();
}