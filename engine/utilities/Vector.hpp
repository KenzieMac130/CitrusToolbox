#pragma once

#include "Common.h"

struct ctVec2 {
	union {
		struct {
			float x;
			float y;
		};
		float data[2];
	};
};

struct ctVec3 {
	union {
		struct {
			float x;
			float y;
			float z;
		};
		float data[3];
	};
};