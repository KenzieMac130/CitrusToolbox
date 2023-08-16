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

#pragma once

#include "Common.h"

#include "cglm/affine.h"
#include "cglm/quat.h"
#include "cglm/cam.h"

/* Hard-coded coordinate spaces (based on glTF) */
// clang-format off
#define CT_UP           0.0f,  1.0f,  0.0f    
#define CT_DOWN         0.0f, -1.0f,  0.0f   
#define CT_FORWARD      0.0f,  0.0f,  1.0f   
#define CT_BACK         0.0f,  0.0f, -1.0f    
#define CT_RIGHT       -1.0f,  0.0f,  0.0f   
#define CT_LEFT         1.0f,  0.0f,  0.0f
// clang-format on
#define CT_VEC3_UP      ctVec3(CT_UP)
#define CT_VEC3_DOWN    ctVec3(CT_DOWN)
#define CT_VEC3_FORWARD ctVec3(CT_FORWARD)
#define CT_VEC3_BACK    ctVec3(CT_BACK)
#define CT_VEC3_RIGHT   ctVec3(CT_RIGHT)
#define CT_VEC3_LEFT    ctVec3(CT_LEFT)

#define CT_VEC3_FROM_2D_GROUND(v) ctVec3(v.x, 0.0f, v.y)
#define CT_VEC3_FROM_2D_FRONT(v)  ctVec3(v.x, v.y, 0.0f)
#define CT_VEC3_FROM_2D_SIDE(v)   ctVec3(0.0f, v.y, v.x)

#define CT_AXIS_UP y
#define CT_AXIS_NS z
#define CT_AXIS_EW x

/* --- Vec2 --- */
struct CT_API CT_ALIGN(CT_ALIGNMENT_VEC2) ctVec2 {
   inline ctVec2() {
      x = 0.0f;
      y = 0.0f;
   }
   inline ctVec2(const float* _p) {
      x = _p[0];
      y = _p[1];
   }
   inline ctVec2(float _v) {
      x = _v;
      y = _v;
   }
   inline ctVec2(float _x, float _y) {
      x = _x;
      y = _y;
   }
   inline ctVec2(struct ctVec3 _v);
   inline ctVec2(struct ctVec4 _v);

   inline ctVec2& operator+=(const ctVec2& v);
   inline ctVec2& operator-=(const ctVec2& v);
   inline ctVec2& operator*=(const ctVec2& v);
   inline ctVec2& operator*=(const float& v);
   inline ctVec2& operator/=(const ctVec2& v);
   inline ctVec2& operator/=(const float& v);

   union {
      struct {
         float x;
         float y;
      };
      float data[2];
   };
};

inline ctVec2 operator+(const ctVec2& a, const ctVec2 b) {
   return ctVec2(a.x + b.x, a.y + b.y);
}

inline ctVec2 operator-(const ctVec2& a, const ctVec2 b) {
   return ctVec2(a.x - b.x, a.y - b.y);
}

inline ctVec2 operator-(const ctVec2& v) {
   return ctVec2(-v.x, -v.y);
}

inline ctVec2 operator*(const ctVec2& a, const ctVec2 b) {
   return ctVec2(a.x * b.x, a.y * b.y);
}

inline ctVec2 operator*(const ctVec2& a, const float v) {
   return ctVec2(a.x * v, a.y * v);
}

inline ctVec2 operator/(const ctVec2& a, const ctVec2 b) {
   return ctVec2(a.x / b.x, a.y / b.y);
}

inline ctVec2 operator/(const ctVec2& a, const float v) {
   return ctVec2(a.x / v, a.y / v);
}

inline ctVec2& ctVec2::operator+=(const ctVec2& v) {
   return *this = *this + v;
}
inline ctVec2& ctVec2::operator-=(const ctVec2& v) {
   return *this = *this - v;
}
inline ctVec2& ctVec2::operator*=(const ctVec2& v) {
   return *this = *this * v;
}
inline ctVec2& ctVec2::operator*=(const float& v) {
   return *this = *this * v;
}
inline ctVec2& ctVec2::operator/=(const ctVec2& v) {
   return *this = *this / v;
}
inline ctVec2& ctVec2::operator/=(const float& v) {
   return *this = *this / v;
}

inline float dot(const ctVec2& a, const ctVec2& b) {
   return a.x * b.x + a.y * b.y;
}

inline float distance(const ctVec2& a, const ctVec2& b) {
   return ctSqrt(ctPow(a.x - b.x, 2.0f) + ctPow(a.y - b.y, 2.0f));
}

inline float length(const ctVec2& v) {
   return ctSqrt(dot(v, v));
}

inline ctVec2 abs(const ctVec2& v) {
   return ctVec2(ctAbs(v.x), ctAbs(v.y));
}

inline ctVec2 normalize(const ctVec2& v) {
   return v / length(v);
}

inline ctVec2 saturate(const ctVec2& v) {
   return ctVec2(ctSaturate(v.x), ctSaturate(v.y));
}

inline ctVec2 lerp(const ctVec2& a, const ctVec2& b, float t) {
   return ctVec2(ctLerp(a.x, b.x, t), ctLerp(a.y, b.y, t));
}

/* --- Vec3 --- */
struct CT_API CT_ALIGN(CT_ALIGNMENT_VEC3) ctVec3 {
   inline ctVec3() {
      x = 0.0f;
      y = 0.0f;
      z = 0.0f;
   }
   inline ctVec3(const float* _p) {
      x = _p[0];
      y = _p[1];
      z = _p[2];
   }
   inline ctVec3(float _v) {
      x = _v;
      y = _v;
      z = _v;
   }
   inline ctVec3(float _x, float _y, float _z) {
      x = _x;
      y = _y;
      z = _z;
   }
   inline ctVec3(struct ctVec2 _v, float z = 0.0f);
   inline ctVec3(struct ctVec4 _v);

   inline ctVec3& operator+=(const ctVec3& v);
   inline ctVec3& operator-=(const ctVec3& v);
   inline ctVec3& operator*=(const ctVec3& v);
   inline ctVec3& operator*=(const float& v);
   inline ctVec3& operator/=(const ctVec3& v);
   inline ctVec3& operator/=(const float& v);

   union {
      struct {
         float x;
         float y;
         float z;
      };
      struct {
         float r;
         float g;
         float b;
      };
      float data[3];
   };
};

inline ctVec3 operator+(const ctVec3& a, const ctVec3 b) {
   return ctVec3(a.x + b.x, a.y + b.y, a.z + b.z);
}

inline ctVec3 operator-(const ctVec3& a, const ctVec3 b) {
   return ctVec3(a.x - b.x, a.y - b.y, a.z - b.z);
}

inline ctVec3 operator-(const ctVec3& v) {
   return ctVec3(-v.x, -v.y, -v.z);
}

inline ctVec3 operator*(const ctVec3& a, const ctVec3 b) {
   return ctVec3(a.x * b.x, a.y * b.y, a.z * b.z);
}

inline ctVec3 operator*(const ctVec3& a, const float v) {
   return ctVec3(a.x * v, a.y * v, a.z * v);
}

inline ctVec3 operator/(const ctVec3& a, const ctVec3 b) {
   return ctVec3(a.x / b.x, a.y / b.y, a.z / b.z);
}

inline ctVec3 operator/(const ctVec3& a, const float v) {
   return ctVec3(a.x / v, a.y / v, a.z / v);
}

inline ctVec3& ctVec3::operator+=(const ctVec3& v) {
   return *this = *this + v;
}
inline ctVec3& ctVec3::operator-=(const ctVec3& v) {
   return *this = *this - v;
}
inline ctVec3& ctVec3::operator*=(const ctVec3& v) {
   return *this = *this * v;
}
inline ctVec3& ctVec3::operator*=(const float& v) {
   return *this = *this * v;
}
inline ctVec3& ctVec3::operator/=(const ctVec3& v) {
   return *this = *this / v;
}
inline ctVec3& ctVec3::operator/=(const float& v) {
   return *this = *this / v;
}

inline float dot(const ctVec3& a, const ctVec3& b) {
   return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline float distance(const ctVec3& a, const ctVec3& b) {
   return ctSqrt(ctPow(a.x - b.x, 2.0f) + ctPow(a.y - b.y, 2.0f) +
                 ctPow(a.z - b.z, 2.0f));
}

inline float length(const ctVec3& v) {
   return ctSqrt(dot(v, v));
}

inline ctVec3 abs(const ctVec3& v) {
   return ctVec3(ctAbs(v.x), ctAbs(v.y), ctAbs(v.z));
}

inline ctVec3 normalize(const ctVec3& v) {
   return v / length(v);
}

inline ctVec3 saturate(const ctVec3& v) {
   return ctVec3(ctSaturate(v.x), ctSaturate(v.y), ctSaturate(v.z));
}

inline ctVec3 cross(const ctVec3& a, const ctVec3& b) {
   // clang-format off
    return ctVec3(
        a.y * b.z - b.y * a.z,
        a.z * b.x - b.z * a.x,
        a.x * b.y - b.x * a.y
    );
   // clang-format on
}

inline ctVec3 lerp(const ctVec3& a, const ctVec3& b, float t) {
   return ctVec3(ctLerp(a.x, b.x, t), ctLerp(a.y, b.y, t), ctLerp(a.z, b.z, t));
}

/* --- Vec4 --- */

struct CT_API CT_ALIGN(CT_ALIGNMENT_VEC4) ctVec4 {
   inline ctVec4() {
      x = 0.0f;
      y = 0.0f;
      z = 0.0f;
      w = 0.0f;
   }
   inline ctVec4(const float* _p) {
      x = _p[0];
      y = _p[1];
      z = _p[2];
      w = _p[3];
   }
   inline ctVec4(float _x, float _y, float _z, float _w) {
      x = _x;
      y = _y;
      z = _z;
      w = _w;
   }
   inline ctVec4(float _v) {
      x = _v;
      y = _v;
      z = _v;
      w = _v;
   }
   inline ctVec4(struct ctVec2 _v, float z = 0.0f, float w = 0.0f);
   inline ctVec4(struct ctVec3 _v, float w = 0.0f);

   inline ctVec4& operator+=(const ctVec4& v);
   inline ctVec4& operator-=(const ctVec4& v);
   inline ctVec4& operator*=(const ctVec4& v);
   inline ctVec4& operator*=(const float& v);
   inline ctVec4& operator/=(const ctVec4& v);
   inline ctVec4& operator/=(const float& v);

   union {
      struct {
         float x;
         float y;
         float z;
         float w;
      };
      struct {
         float r;
         float g;
         float b;
         float a;
      };
      float data[4];
   };
};

inline ctVec4 operator+(const ctVec4& a, const ctVec4 b) {
   return ctVec4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}

inline ctVec4 operator-(const ctVec4& a, const ctVec4 b) {
   return ctVec4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}

inline ctVec4 operator-(const ctVec4& v) {
   return ctVec4(-v.x, -v.y, -v.z, -v.w);
}

inline ctVec4 operator*(const ctVec4& a, const ctVec4 b) {
   return ctVec4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
}

inline ctVec4 operator*(const ctVec4& a, const float v) {
   return ctVec4(a.x * v, a.y * v, a.z * v, a.w * v);
}

inline ctVec4 operator/(const ctVec4& a, const ctVec4 b) {
   return ctVec4(a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w);
}

inline ctVec4 operator/(const ctVec4& a, const float v) {
   return ctVec4(a.x / v, a.y / v, a.z / v, a.w / v);
}

inline ctVec4& ctVec4::operator+=(const ctVec4& v) {
   return *this = *this + v;
}
inline ctVec4& ctVec4::operator-=(const ctVec4& v) {
   return *this = *this - v;
}
inline ctVec4& ctVec4::operator*=(const ctVec4& v) {
   return *this = *this * v;
}
inline ctVec4& ctVec4::operator*=(const float& v) {
   return *this = *this * v;
}
inline ctVec4& ctVec4::operator/=(const ctVec4& v) {
   return *this = *this / v;
}
inline ctVec4& ctVec4::operator/=(const float& v) {
   return *this = *this / v;
}

inline float dot(const ctVec4& a, const ctVec4& b) {
   return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

inline float distance(const ctVec4& a, const ctVec4& b) {
   return ctSqrt(ctPow(a.x - b.x, 2.0f) + ctPow(a.y - b.y, 2.0f) +
                 ctPow(a.z - b.z, 2.0f) + ctPow(a.w - b.w, 2.0f));
}

inline float length(const ctVec4& v) {
   return ctSqrt(dot(v, v));
}

inline ctVec4 abs(const ctVec4& v) {
   return ctVec4(ctAbs(v.x), ctAbs(v.y), ctAbs(v.z), ctAbs(v.w));
}

inline ctVec4 normalize(const ctVec4& v) {
   return v / length(v);
}

inline ctVec4 saturate(const ctVec4& v) {
   return ctVec4(ctSaturate(v.x), ctSaturate(v.y), ctSaturate(v.z), ctSaturate(v.w));
}

inline ctVec4 lerp(const ctVec4& a, const ctVec4& b, float t) {
   return ctVec4(
     ctLerp(a.x, b.x, t), ctLerp(a.y, b.y, t), ctLerp(a.z, b.z, t), ctLerp(a.w, b.w, t));
}

/* --- Colors --- */

enum ctColorComponents {
   CT_COLOR_COMPONENT_NONE = 0x00,
   CT_COLOR_COMPONENT_R = 0x01,
   CT_COLOR_COMPONENT_G = 0x02,
   CT_COLOR_COMPONENT_B = 0x04,
   CT_COLOR_COMPONENT_A = 0x08,
   CT_COLOR_COMPONENT_RG = CT_COLOR_COMPONENT_R | CT_COLOR_COMPONENT_G,
   CT_COLOR_COMPONENT_RGB = CT_COLOR_COMPONENT_RG | CT_COLOR_COMPONENT_B,
   CT_COLOR_COMPONENT_RGBA = CT_COLOR_COMPONENT_RGB | CT_COLOR_COMPONENT_A,
};

#define CT_COLOR_BLACK           ctVec4(0.0f, 0.0f, 0.0f, 1.0f)
#define CT_COLOR_GREY            ctVec4(0.5f, 0.5f, 0.5f, 1.0f)
#define CT_COLOR_WHITE           ctVec4(1.0f, 1.0f, 1.0f, 1.0f)
#define CT_COLOR_INVISIBLE_BLACK ctVec4(0.0f, 0.0f, 0.0f, 0.0f)
#define CT_COLOR_INVISIBLE_GREY  ctVec4(0.5f, 0.5f, 0.5f, 0.0f)
#define CT_COLOR_INVISIBLE_WHITE ctVec4(1.0f, 1.0f, 1.0f, 0.0f)
#define CT_COLOR_RED             ctVec4(1.0f, 0.0f, 0.0f, 1.0f)
#define CT_COLOR_GREEN           ctVec4(0.0f, 1.0f, 0.0f, 1.0f)
#define CT_COLOR_BLUE            ctVec4(0.0f, 0.0f, 1.0f, 1.0f)
#define CT_COLOR_YELLOW          ctVec4(1.0f, 1.0f, 0.0f, 1.0f)
#define CT_COLOR_ORANGE          ctVec4(1.0f, 0.5f, 0.0f, 1.0f)
#define CT_COLOR_PURPLE          ctVec4(0.5f, 0.0f, 1.0f, 1.0f)
#define CT_COLOR_PINK            ctVec4(1.0f, 0.0f, 1.0f, 1.0f)

/* --- Bounding Box --- */

struct CT_API ctBoundBox {
   inline ctBoundBox() {
      min = ctVec3(FLT_MAX);
      max = ctVec3(-FLT_MAX);
   }
   inline ctBoundBox(ctVec3 pmin, ctVec3 pmax) {
      min = pmin;
      max = pmax;
   }

   inline void AddPoint(ctVec3 pt) {
      if (pt.x < min.x) { min.x = pt.x; }
      if (pt.y < min.y) { min.y = pt.y; }
      if (pt.z < min.z) { min.z = pt.z; }
      if (pt.x > max.x) { max.x = pt.x; }
      if (pt.y > max.y) { max.y = pt.y; }
      if (pt.z > max.z) { max.z = pt.z; }
   }

   inline void AddBox(ctBoundBox bx) {
      AddPoint(bx.min);
      AddPoint(bx.max);
   }

   inline bool isValid() {
      return min.x <= max.x && min.y <= max.y && min.z <= max.z;
   }

   inline ctVec3 NormalizeVector(ctVec3 vector) const {
      return (vector - min) / (max - min);
   }

   inline ctVec3 DeNormalizeVector(ctVec3 vector) const {
      return vector * (max - min) + min;
   }

   inline struct ctBoundSphere ToSphere();

   ctVec3 min;
   ctVec3 max;
};

struct CT_API ctBoundBox2D {
   inline ctBoundBox2D() {
      min = ctVec2(FLT_MAX);
      max = ctVec2(-FLT_MAX);
   }
   inline ctBoundBox2D(ctVec2 pmin, ctVec2 pmax) {
      min = pmin;
      max = pmax;
   }

   inline void AddPoint(ctVec2 pt) {
      if (pt.x < min.x) { min.x = pt.x; }
      if (pt.y < min.y) { min.y = pt.y; }
      if (pt.x > max.x) { max.x = pt.x; }
      if (pt.y > max.y) { max.y = pt.y; }
   }

   inline void AddBox(ctBoundBox2D bx) {
      AddPoint(bx.min);
      AddPoint(bx.max);
   }

   inline bool isValid() {
      return min.x <= max.x && min.y <= max.y;
   }

   inline ctVec2 NormalizeVector(ctVec2 vector) const {
      return (vector - min) / (max - min);
   }

   inline ctVec2 DeNormalizeVector(ctVec2 vector) const {
      return vector * (max - min) + min;
   }

   ctVec2 min;
   ctVec2 max;
};

/* --- Sphere --- */

struct CT_API ctBoundSphere {
   inline ctBoundSphere() {
      position = ctVec3();
      radius = -FLT_MAX;
   }
   inline ctBoundSphere(ctVec3 p, float r) {
      position = p;
      radius = r;
   }
   inline ctBoundSphere(ctBoundBox box) {
      if (!box.isValid()) {
         position = ctVec3();
         radius = -FLT_MAX;
         return;
      }
      position = (box.min + box.max) / 2.0f;
      AddBox(box);
   }
   inline void AddPoint(ctVec3 pt) {
      const float dist = distance(pt, position);
      if (dist > radius) { radius = dist; }
   }
   inline void AddBox(ctBoundBox box) {
      if (!box.isValid()) { return; };
      AddPoint(box.min);
      AddPoint(box.max);
   }

   inline ctBoundBox ToBox() {
      const ctVec3 bmin = {position.x - radius, position.y - radius, position.z - radius};
      const ctVec3 bmax = {position.x + radius, position.y + radius, position.z + radius};
      return ctBoundBox(bmin, bmax);
   }

   inline bool isValid() {
      return radius >= 0.0f;
   }

   ctVec3 position;
   float radius;
};

inline struct ctBoundSphere ctBoundBox::ToSphere() {
   return ctBoundSphere(*this);
}

/* --- Quaternion --- */

struct CT_API CT_ALIGN(CT_ALIGNMENT_QUAT) ctQuat {
   inline ctQuat() {
      x = 0.0f;
      y = 0.0f;
      z = 0.0f;
      w = 1.0f;
   }
   inline ctQuat(const float* _p) {
      x = _p[0];
      y = _p[1];
      z = _p[2];
      w = _p[3];
   }
   inline ctQuat(float _x, float _y, float _z, float _w) {
      x = _x;
      y = _y;
      z = _z;
      w = _w;
   }
   inline ctQuat(const ctVec3& axis, float angle) {
      glm_quatv(data, angle, (float*)axis.data);
   }
   inline ctQuat(float _v) {
      x = _v;
      y = _v;
      z = _v;
      w = _v;
   }
   inline ctQuat(struct ctVec4 _v);

   inline ctQuat& operator*=(const ctQuat& q) {
      glm_quat_mul((float*)q.data, data, data);
      return *this;
   }

   inline ctVec3 getForward() const;
   inline ctVec3 getBack() const;
   inline ctVec3 getUp() const;
   inline ctVec3 getDown() const;
   inline ctVec3 getRight() const;
   inline ctVec3 getLeft() const;

   union {
      struct {
         float x;
         float y;
         float z;
         float w;
      };
      float data[4];
   };
};

inline ctQuat operator*(const ctQuat& a, const ctQuat b) {
   ctQuat result;
   glm_quat_mul((float*)a.data, (float*)b.data, result.data);
   return result;
}

inline ctQuat operator-(const ctQuat& v) {
   ctQuat result;
   glm_quat_inv((float*)v.data, result.data);
   return result;
}

inline ctQuat normalize(const ctQuat& v) {
   ctQuat result;
   glm_quat_normalize_to((float*)v.data, result.data);
   return result;
}

inline ctVec3 operator*(const ctVec3& v, const ctQuat q) {
   ctVec3 result;
   glm_quat_rotatev((float*)q.data, (float*)v.data, (float*)result.data);
   return result;
}

inline ctQuat ctQuatLookTowards(const ctVec3 dir,
                                const ctVec3 fwd = CT_VEC3_FORWARD,
                                const ctVec3 up = CT_VEC3_UP) {
   ctQuat result;
   glm_quat_for((float*)dir.data, (float*)fwd.data, (float*)up.data, (float*)result.data);
   return result;
}

inline ctQuat ctQuatSlerp(const ctQuat& a, const ctQuat& b, float t) {
   ctQuat result;
   glm_quat_slerp((float*)a.data, (float*)b.data, t, result.data);
   return result;
}

inline ctVec3 ctQuat::getForward() const {
   return CT_VEC3_FORWARD * *this;
}
inline ctVec3 ctQuat::getBack() const {
   return CT_VEC3_BACK * *this;
}
inline ctVec3 ctQuat::getUp() const {
   return CT_VEC3_UP * *this;
}
inline ctVec3 ctQuat::getDown() const {
   return CT_VEC3_DOWN * *this;
}
inline ctVec3 ctQuat::getRight() const {
   return CT_VEC3_RIGHT * *this;
}
inline ctVec3 ctQuat::getLeft() const {
   return CT_VEC3_LEFT * *this;
}

inline ctQuat ctQuatYawPitchRoll(float yaw, float pitch, float roll) {
   ctQuat result = ctQuat();
   result *= ctQuat(CT_VEC3_UP, yaw);
   result *= ctQuat(CT_VEC3_RIGHT, pitch);
   result *= ctQuat(CT_VEC3_FORWARD, roll);
   return result;
}

/* --- Transforms ---- */

struct CT_API ctTransform {
   inline ctTransform() {
      translation = ctVec3();
      rotation = ctQuat();
      scale = ctVec3(1.0f);
   }
   inline ctTransform(ctVec3 p) {
      translation = p;
      rotation = ctQuat();
      scale = ctVec3(1.0f);
   };
   inline ctTransform(ctVec3 p, ctQuat q) {
      translation = p;
      rotation = q;
      scale = ctVec3(1.0f);
   };
   inline ctTransform(ctVec3 p, ctQuat q, ctVec3 s) {
      translation = p;
      rotation = q;
      scale = s;
   };

   ctVec3 translation;
   ctQuat rotation;
   ctVec3 scale;
};

/* --- Mat4 --- */
struct CT_API CT_ALIGN(CT_ALIGNMENT_MAT4) ctMat4 {
   inline ctMat4() {
      // clang-format off
       data[0][0] = 0; data[0][1] = 0; data[0][2] = 0; data[0][3] = 0;
       data[1][0] = 0; data[1][1] = 0; data[1][2] = 0; data[1][3] = 0;
       data[2][0] = 0; data[2][1] = 0; data[2][2] = 0; data[2][3] = 0;
       data[3][0] = 0; data[3][1] = 0; data[3][2] = 0; data[3][3] = 0;
      // clang-format on
   }
   inline ctMat4(float _d) {
      // clang-format off
       data[0][0] = _d; data[0][1] =  0; data[0][2] =  0; data[0][3] =  0;
       data[1][0] =  0; data[1][1] = _d; data[1][2] =  0; data[1][3] =  0;
       data[2][0] =  0; data[2][1] =  0; data[2][2] = _d; data[2][3] =  0;
       data[3][0] =  0; data[3][1] =  0; data[3][2] =  0; data[3][3] = _d;
      // clang-format on
   }
   inline ctMat4(
     // clang-format off
       float v00, float v01, float v02, float v03,
       float v10, float v11, float v12, float v13,
       float v20, float v21, float v22, float v23,
       float v30, float v31, float v32, float v33
     // clang-format on
   ) {
      // clang-format off
       data[0][0] = v00; data[0][1] = v01; data[0][2] = v02; data[0][3] = v03;
       data[1][0] = v10; data[1][1] = v11; data[1][2] = v12; data[1][3] = v13;
       data[2][0] = v20; data[2][1] = v21; data[2][2] = v22; data[2][3] = v23;
       data[3][0] = v30; data[3][1] = v31; data[3][2] = v32; data[3][3] = v33;
      // clang-format on
   }
   inline ctMat4(const float* values) {
      memcpy(&data[0][0], values, sizeof(float) * 4 * 4);
   }
   inline float& operator()(int r, int c) {
      return data[r][c];
   }
   inline float operator()(int r, int c) const {
      return data[r][c];
   }

   inline ctMat4& operator*=(const ctMat4& v);

   float data[4][4];
};

inline ctMat4 operator*(const ctMat4& a, const ctMat4 b) {
   ctMat4 result;
   glm_mat4_mul((vec4*)a.data, (vec4*)b.data, result.data);
   return result;
}

inline ctVec4 operator*(const ctVec4 v, const ctMat4& m) {
   ctVec4 result;
   glm_mat4_mulv((vec4*)m.data, (float*)v.data, result.data);
   return result;
}

inline ctVec3 operator*(const ctVec3 v, const ctMat4& m) {
   ctVec3 result;
   glm_mat4_mulv3((vec4*)m.data, (float*)v.data, 1.0f, result.data);
   return result;
}

inline ctMat4& ctMat4::operator*=(const ctMat4& v) {
   return *this = *this * v;
}

inline ctMat4 ctMat4Identity() {
   return ctMat4(1.0f);
}

inline void ctMat4Translate(ctMat4& m, const ctVec3 v) {
   glm_translate(m.data, (float*)v.data);
}

inline void ctMat4Rotate(ctMat4& m, const ctQuat q) {
   glm_quat_rotate(m.data, (float*)q.data, m.data);
}

inline void ctMat4Scale(ctMat4& m, const ctVec3 v) {
   glm_scale(m.data, (float*)v.data);
}

inline void ctMat4Scale(ctMat4& m, float s) {
   glm_scale_uni(m.data, s);
}

inline void ctMat4FromTransform(ctMat4& m, const ctTransform& transform) {
   m = ctMat4Identity();
   ctMat4Translate(m, transform.translation);
   ctMat4Rotate(m, transform.rotation);
   ctMat4Scale(m, transform.scale);
}

inline void ctMat4RemoveTranslation(ctMat4& m) {
   m.data[3][0] = 0.0f;
   m.data[3][1] = 0.0f;
   m.data[3][2] = 0.0f;
   m.data[3][3] = 1.0f;
}

/* By using this function you acknowledge loss of precision */
/* NEVER USE ON WORLD SPACE!!! */
inline ctMat4 ctMat4InverseLossy(ctMat4 m) {
   ctMat4 result;
   glm_mat4_inv(m.data, result.data);
   return result;
}

/* By using this function you acknowledge you will never truely recover the original
 * transform */
inline void
ctMat4AwkwardDecompose(ctMat4 m, ctVec3& translation, ctQuat& rotation, ctVec3& scale) {
   ctMat4 rotmat;
   ctVec4 translate4d;
   glm_decompose(m.data, translate4d.data, rotmat.data, scale.data);
   glm_mat4_quat(rotmat.data, rotation.data);
   translation = ctVec3(translate4d);
}

/* By using this function you acknowledge you will never truely recover the original
 * transform */
inline void ctMat4AwkwardDecompose(ctMat4 m, ctTransform& transform) {
   ctMat4AwkwardDecompose(m, transform.translation, transform.rotation, transform.scale);
}

inline void
ctMat4PerspectiveInfinite(ctMat4& m, float fov, float aspect, float nearClip) {
   glm_perspective_infinite(fov, aspect, nearClip, (vec4*)m.data);
#ifdef CITRUS_GFX_VULKAN
   m.data[2][3] = 1.0f;
   m.data[0][0] *= -1.0f;
   m.data[1][1] *= -1.0f;
#endif
}

inline void ctMat4Ortho(ctMat4& m, float size, float aspect) {
   glm_ortho_default_s(aspect, size, m.data);
#ifdef CITRUS_GFX_VULKAN
   m.data[0][0] *= -1.0f;
   m.data[1][1] *= -1.0f;
#endif
}

inline ctMat4 ctMat4FromQuat(ctQuat q) {
   ctMat4 mat = ctMat4Identity();
   ctMat4Rotate(mat, q);
   return mat;
}

/* --- Camera --- */
struct CT_API ctCameraInfo {
   inline ctCameraInfo() {
      position = ctVec3();
      rotation = ctQuat();
      fov = 0.785f;
      aspectRatio = 1.0f;
      nearClip = 0.01f;
   }
   ctVec3 position;
   ctQuat rotation;
   float fov;
   float aspectRatio;
   float nearClip;

   inline ctMat4 GetViewMatrix() {
      ctMat4 result = ctMat4Identity();
      ctMat4Rotate(result, -rotation);
      ctMat4Translate(result, -position);
      return result;
   }
   inline ctMat4 GetInverseProjectionMatrix() {
      ctMat4 result = ctMat4Identity();
      ctMat4PerspectiveInfinite(result, fov, aspectRatio, nearClip);
      return result;
   }
   inline ctMat4 GetViewInverseProjectionMatrix() {
      return GetInverseProjectionMatrix() * GetViewMatrix();
   }
   inline ctVec3 ProjectionToWorld(ctVec4 projectionCoord) {
      ctMat4 matrix = ctMat4InverseLossy(GetViewInverseProjectionMatrix());
      ctMat4Translate(matrix, position);
      ctMat4Rotate(matrix, rotation);
      return ctVec3(projectionCoord * matrix);
   }
   /* Viewport Coord (XY) Depth (Z) Visible Sign (W) */
   inline ctVec4 WorldToProjection(ctVec3 worldPosition) {
      return worldPosition * GetViewInverseProjectionMatrix();
   }
   inline ctVec3 ScreenToWorld(ctVec2 normalizedScreenCoord, float depth = 1.0f) {
      ctVec2 projCoord = normalizedScreenCoord;
      projCoord *= 2.0f;
      projCoord += 0.5f;
      return ProjectionToWorld(ctVec4(projCoord.x, projCoord.y, depth, 1.0f));
   }
   /* Normalized Screen (XY) Depth (Z) Visible Sign (W) */
   inline ctVec4 WorldToScreen(ctVec3 worldPosition) {
      ctVec4 proj = WorldToProjection(worldPosition);
      proj.x *= 0.5f;
      proj.y *= 0.5f;
      proj.x -= 0.5f;
      proj.y -= 0.5f;
      return proj;
   }
};

/* --- Narrowing/Widening --- */
inline ctVec2::ctVec2(struct ctVec3 _v) {
   x = _v.x;
   y = _v.y;
}
inline ctVec2::ctVec2(struct ctVec4 _v) {
   x = _v.x;
   y = _v.y;
}
inline ctVec3::ctVec3(struct ctVec2 _v, float _z) {
   x = _v.x;
   y = _v.y;
   z = _z;
}
inline ctVec3::ctVec3(struct ctVec4 _v) {
   x = _v.x;
   y = _v.y;
   z = _v.z;
}
inline ctVec4::ctVec4(struct ctVec2 _v, float _z, float _w) {
   x = _v.x;
   y = _v.y;
   z = _z;
   w = _w;
}
inline ctVec4::ctVec4(struct ctVec3 _v, float _w) {
   x = _v.x;
   y = _v.y;
   z = _v.z;
   w = _w;
}
inline ctQuat::ctQuat(struct ctVec4 _v) {
   x = _v.x;
   y = _v.y;
   z = _v.z;
   w = _v.w;
}

/* --- Middleware Conversion --- */
#define ctVec2ToImGui(v) ImVec2(v.x, v.y)
#define ctVec3ToImGui(v) ImVector(v.x, v.y, v.z)
#define ctVec4ToImGui(v) ImVec4(v.x, v.y, v.z, v.w)

#define ctVec2ToIm3d(v)      Im3d::Vec2(v.x, v.y)
#define ctVec3ToIm3d(v)      Im3d::Vec3(v.x, v.y, v.z)
#define ctVec4ToIm3d(v)      Im3d::Vec4(v.x, v.y, v.z, v.w)
#define ctVec4ToIm3dColor(v) Im3d::Color(v.r, v.g, v.b, v.a)
#define ctMat4ToIm3d(v)                                                                  \
   Im3d::Mat4(v.data[0][0],                                                              \
              v.data[1][0],                                                              \
              v.data[2][0],                                                              \
              v.data[3][0],                                                              \
              v.data[0][1],                                                              \
              v.data[1][1],                                                              \
              v.data[2][1],                                                              \
              v.data[3][1],                                                              \
              v.data[0][2],                                                              \
              v.data[1][2],                                                              \
              v.data[2][2],                                                              \
              v.data[3][2],                                                              \
              v.data[0][3],                                                              \
              v.data[1][3],                                                              \
              v.data[2][3],                                                              \
              v.data[3][3])

#define ctVec2FromIm3d(v)      ctVec2(v.x, v.y)
#define ctVec3FromIm3d(v)      ctVec3(v.x, v.y, v.z)
#define ctVec4FromIm3d(v)      ctVec4(v.x, v.y, v.z, v.w)
#define ctVec4FromIm3dColor(v) ctVec4(v.r, v.g, v.b, v.a)
#define ctMat4FromIm3d(v)                                                                \
   ctMat4(v(0, 0),                                                                       \
          v(0, 1),                                                                       \
          v(0, 2),                                                                       \
          v(0, 3),                                                                       \
          v(1, 0),                                                                       \
          v(1, 1),                                                                       \
          v(1, 2),                                                                       \
          v(1, 3),                                                                       \
          v(2, 0),                                                                       \
          v(2, 1),                                                                       \
          v(2, 2),                                                                       \
          v(2, 3),                                                                       \
          v(3, 0),                                                                       \
          v(3, 1),                                                                       \
          v(3, 2),                                                                       \
          v(3, 3))