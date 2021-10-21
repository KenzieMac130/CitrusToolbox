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

#define CT_MATERIAL_START\
CT_STORAGE_BUFFER_ARRAY(sMaterialBuffer, material, {\

#define CT_MATERIAL_FLOAT(_NAME, _DEFAULT) float _NAME;
#define CT_MATERIAL_VECTOR(_NAME, _DEFAULT) vec4 _NAME;
#define CT_MATERIAL_TEXTURE(_NAME, _DEFAULT) int _NAME;

#define CT_MATERIAL_END })