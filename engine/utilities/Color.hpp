#pragma once

#include "Common.h"

struct ctColorU8 {
	union {
		struct {
			uint8_t r;
			uint8_t g;
			uint8_t b;
		};
		uint8_t data[3];
	};
};

struct ctColorF32 {
	union {
		struct {
			float r;
			float g;
			float b;
		};
		float data[3];
	};
};