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

enum ctWADProtoHeaderFlags {
	CT_WADPROTO_HEADER_FLAG_HAS_GPU_FILE = 0x001
};

#define CT_WADPROTO_NAME_HEADER {'C','I','T','R','U','S'}
#define CT_WADPROTO_HEADER_MAGIC 0x52544943
#define CT_WADPROTO_HEADER_INTERNAL_REV 1

struct CT_API ctWADProtoHeader {
	int32_t magic;
	int32_t revision;
	int32_t flags;
	int32_t gpuFilePath;
};