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

#include "utilities/Common.h"

/* must match ctGPUExternalTextureType */
enum ctImageType {
   CT_IMAGE_TYPE_1D,
   CT_IMAGE_TYPE_2D,
   CT_IMAGE_TYPE_3D,
   CT_IMAGE_TYPE_CUBE
};

class ctImage {
public:
   ctImage();
   ctImage(ctFile& file);
   ~ctImage();

   ctResults Create(uint32_t iwidth,
                    uint32_t iheight,
                    uint32_t idepth,
                    uint32_t imips,
                    enum TinyImageFormat iformat,
                    ctImageType itype,
                    size_t* pLevelSizes);

   /* supported formats: ktx, dds, anything from stb_image */
   ctResults Load(ctFile& file);

   /* supported formats: png, jpg, tga, bmp, ktx, dds
   cubes, depth, and mips:
      only ktx and dds support these features
      other formats will only export the first mip of a 1D/2D texture
   color handling:
      color space transforms will not be performed! (ex: Linear->sRGB)
      color depth conversions will be handled by scalarizing and clipping
      ktx and dds: full color is preserved
      hdr: partial color depth is preserved
      other formats: non-compressed colors will be converted to 8-Bit */
   ctResults Save(ctFile& file, const char* format = "png");

   /* cannot convert compressed or packed formats */
   ctResults ConvertFormat(enum TinyImageFormat newFormat);
   /* only supports resizing 2D images (invalidates mips) */
   ctResults Resize(uint32_t newWidth,
                    uint32_t newHeight,
                    bool wrap = true,
                    bool nearest = false);
   /* generate mipmaps for 2D images (this is slow! 0=full chain) */
   ctResults GenerateMipmaps(uint32_t mipCount = 0);

   /* slow! only availible in offline builds, meant for compiling embedded images */
   ctResults CompileOffline(const ctStringUtf8& filePath,
                            enum TinyImageFormat outFormat,
                            uint32_t outMipCount = 0);

   bool isValid() const;
   inline uint32_t GetWidth() const {
      return width;
   }
   inline uint32_t GetHeight() const {
      return height;
   }
   inline uint32_t GetDepth() const {
      return depth;
   }
   inline uint32_t GetMipCount() const {
      return mips;
   }
   inline enum TinyImageFormat GetFormat() const {
      return format;
   }
   inline ctImageType GetType() const {
      return type;
   }
   inline uint32_t GetSliceCount() const {
      switch (GetType()) {
         case CT_IMAGE_TYPE_3D: return 1;
         case CT_IMAGE_TYPE_CUBE: return GetDepth() * 6;
         default: return GetDepth();
      }
   }
   inline uint32_t GetArrayLayerCount() const {
      switch (GetType()) {
         case CT_IMAGE_TYPE_3D: return 1;
         default: return GetDepth();
      }
   }
   inline size_t GetByteCount(uint32_t mipLevel = 0) const {
      ctAssert(mipLevel < mips);
      return levelSizes[mipLevel];
   }
   inline const void* GetData(uint32_t mipLevel = 0) const {
      ctAssert(mipLevel < mips);
      return levels[mipLevel];
   }
   inline void* GetData(uint32_t mipLevel = 0) {
      ctAssert(mipLevel < mips);
      return levels[mipLevel];
   }
   inline void GetMipDimensions(uint32_t mipLevel = 0,
                         uint32_t* pOutWidth = NULL,
                         uint32_t* pOutHeight = NULL,
                         uint32_t* pOutDepth = NULL) const {
      GetMipDimensions(GetWidth(), GetHeight(), GetDepth(), mipLevel, pOutWidth, pOutHeight, pOutDepth);
   }
   static void GetMipDimensions(uint32_t inWidth,
                                uint32_t inHeight,
                                uint32_t inDepth,
                                uint32_t mipLevel = 0,
                                uint32_t* pOutWidth = NULL,
                                uint32_t* pOutHeight = NULL,
                                uint32_t* pOutDepth = NULL);

   bool CanSample();
   ctVec4 SampleLinear(ctVec2 uv);
   ctVec4 SampleNearest(ctVec2 uv);
   ctVec4 SampleNearest(uint32_t x, uint32_t y);

private:
   void Invalidate();
   void MakeCustom();
   void Release();
   void ReleaseLoader();
   ctResults LoadKTX(ctFile& file);
   ctResults LoadDDS(ctFile& file);
   ctResults LoadMisc(ctFile& file);

   enum ctImageSource source;
   uint32_t width;
   uint32_t height;
   uint32_t depth;
   uint32_t mips;
   enum TinyImageFormat format;
   ctImageType type;
   size_t levelSizes[CT_MAX_MIP_LEVELS];
   void* levels[CT_MAX_MIP_LEVELS];
   void* loaderdata;
};