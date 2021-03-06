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

#extension GL_EXT_nonuniform_qualifier : require

layout(set = 0, binding = 0) uniform sampler bindlessSamplers[];

layout(set = 0, binding = 1) uniform texture1D bindlessSampledImages1D[];
layout(set = 0, binding = 1) uniform texture2D bindlessSampledImages2D[];
layout(set = 0, binding = 1) uniform texture3D bindlessSampledImages3D[];
layout(set = 0, binding = 1) uniform textureCube bindlessSampledImagesCube[];

layout(set = 0, binding = 2) uniform writeonly image1D bindlessStorageImages1D[];
layout(set = 0, binding = 2) uniform writeonly image2D bindlessStorageImages2D[];
layout(set = 0, binding = 2) uniform writeonly image3D bindlessStorageImages3D[];
layout(set = 0, binding = 2) uniform writeonly imageCube bindlessStorageImagesCube[];

#define CT_STORAGE_BUFFER_ARRAY(_BUFFNAME, _NAME, _CONTENTS) layout(set = 0, binding = 3) buffer _BUFFNAME _CONTENTS _NAME[];