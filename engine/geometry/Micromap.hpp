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

/* see https://developer.download.nvidia.com/ProGraphics/nvpro-samples/slides/Micro-Mesh_Basics.pdf */
struct ctGeometryBakerMicromapFace {
    inline ctGeometryBakerMicromapFace() {
        subdivLevel = 8;
        memset(data, 0, sizeof(data));
    }
    /* feed the triangle uv coordinates here to bake the micromap */
    void Bake(const ctImage* pImage, 
              const ctVec2 uvs[3],
              const uint32_t channel = 0,
              float* pOutMin = NULL,
              float* pOutMax = NULL);
    /* use the "bird curve" to get a micro-vertex index */
    /* todo: see
    https://github.com/NVIDIAGameWorks/Displacement-MicroMap-BaryFile/blob/ed72ae078f405db9b4268d2c0ecc87c3b6d2b558/src/bary_core.cpp#L189 */
    uint32_t BarycentricToIndex(const ctVec2 st);
    ctVec2 IndexToBarycentric(const uint32_t index); /* todo: just use average lookup tables from above result */
    inline void Set(uint8_t value, uint32_t coordIdx) {
        ctAssert(coordIdx < 55);
        data[coordIdx] = value;
    }
    inline uint8_t Get(uint32_t coordIdx) {
        ctAssert(coordIdx < 55);
        return data[coordIdx];
    }
    uint8_t subdivLevel; /* maximum of 8 */
    uint8_t data[55]; /* microvertex in bird curve only */
};

/* see ctGeometryTriangleMaterialBlend for more info */
struct ctGeometryBakerTriangleMaterialBlend {
    /* when merging the new material layer we determine where it would go in a temporary 5 channel
    version of the stack based on its position in the material offsets, then any layers below it get
    checked for complete occlusion, if so then the bottom layer becomes the next layer up, we repeat
    this until there is no more collapsing bottom layers, think of this as a sort of a game of 
    "tetris". if the amount of layers remaining after merging is less than or equal to 4 then we
    are finished with the merge, otherwise its onto step 2: evicting the mask with the least coverage
    and once that is done we will have our finished stack */
    void MergeLayer(const ctGeometryBakerMicromapFace& micromap, uint8_t subMaterialIndex);
    uint8_t materialOffsets[4];
    ctGeometryBakerMicromapFace micromapMask[3];
};

/* material blending is handled by using blend masks to affect
a single triange, a triangle can have up to 4 materials mapped to it
these materials are determined by the materialOffsets
the micromap masks are then used to encode the blend mask into the
face itself using a tesselated pattern as a "micromap"
each triangle is given 64 sample points at 2^4 precision with only
roughly 33 bytes per triangle (similar to uncompressed skinning!)
while using even low resolution textures produces a much higher footprint
in addition each triangle can address a unique material set making
it possible to blend extremely large terrain elements at a low cost */
struct ctGeometryTriangleMaterialBlend {
    uint8_t materialSlotIndices[4];
    uint32_t micromapSubdivLevel;
    uint32_t micromapBitOffset; /* 3 continuous micromaps expected */
};

class ctGeometryBakerMicromapGenerator {
    /* generate material blending from greyscale masks representing scalar material slot coverage */
    ctResults GenerateMaterialBlending(ctFile& outFile,
                                       const ctGeometryBakerSubmesh& submesh,
                                       const ctDynamicArray<uint32_t>& materialSlot,
                                       const ctDynamicArray<class ctImage*> slotMasks,
                                       size_t uvIndex = 0);
};