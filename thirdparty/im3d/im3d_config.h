#pragma once

#include "../engine/utilities/Common.h"

// User-defined assertion handler (default is cassert assert()).
//#define IM3D_ASSERT(e) assert(e)

// User-defined malloc/free. Define both or neither (default is cstdlib malloc()/free()).
#define IM3D_MALLOC(size) ctMalloc(size)
#define IM3D_FREE(ptr) ctFree(ptr)

// User-defined API declaration (e.g. __declspec(dllexport)).
//#define IM3D_API

// Use a thread-local context pointer.
//#define IM3D_THREAD_LOCAL_CONTEXT_PTR 1

// Use row-major internal matrix layout.
//#define IM3D_MATRIX_ROW_MAJOR 1

// Force vertex data alignment (default is 4 bytes).
#define IM3D_VERTEX_ALIGNMENT 32

// Enable internal culling for primitives (everything drawn between Begin*()/End()). The application must set a culling frustum via AppData.
#define IM3D_CULL_PRIMITIVES 0

// Enable internal culling for gizmos. The application must set a culling frustum via AppData.
#define IM3D_CULL_GIZMOS 0

// Conversion to/from application math types.
//#define IM3D_VEC2_APP \
//	Vec2(const ctVec2& _v)			{ ctVec2ToIm3d(_v); };\
//	operator ctVec2() const         { return ctVec3(x, y); };
//#define IM3D_VEC3_APP\
//	Vec3(const ctVec3& _v)			{ ctVec3ToIm3d(_v); };\
//	operator ctVec3() const         { return ctVec3(x, y, z); };
//#define IM3D_VEC4_APP\
//	Vec3(const ctVec4& _v)			{ ctVec4ToIm3d(_v); };\
//	operator ctVec4() const         { return ctVec3(x, y, z, w); };
////#define IM3D_MAT3_APP\
////	Mat3(const glm::mat3& _m)          { for (int i = 0; i < 9; ++i) m[i] = *(&(_m[0][0]) + i); }\
////	operator glm::mat3() const         { glm::mat3 ret; for (int i = 0; i < 9; ++i) *(&(ret[0][0]) + i) = m[i]; return ret; }
//#define IM3D_MAT4_APP\
//	Mat4(const ctMat4& _v)			{ ctMat4ToIm3d(_v); }\
////	operator ctMat4() const         { return ctMat4FromIm3d(*this); }