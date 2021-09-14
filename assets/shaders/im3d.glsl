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

#define CT_IMPLEMENTATION

#include "utilities/common.glsl"

#ifdef VERTEX_SHADER
struct sVertexData {
    vec3 pos;
    float scale;
    uint icolor;
};

CT_STORAGE_BUFFER_ARRAY(sVertexBuffer, vertexBuffers, {
    sVertexData data[];
})

layout(push_constant) uniform sPushConstant {
    mat4 viewProj;
    vec2 viewSize;
    uint vertBuffBindIdx;
    uint primType;
} drawData;

vec2 quadVerts[] = {
    vec2(1,-1),
    vec2(1,1),
    vec2(-1,1),
    vec2(-1,-1),
    vec2(1,-1),
    vec2(-1,1)
};

int lineIdx[] = {
0,1,0,0,1,1
};

layout(location = 0) out vec4 outColor;

void main(){
    uint lineStart = 0;
    uint lineEnd = 0;
    uint vtxIdx = 0;
    /* Triangles */
    if(drawData.primType == 0) {
        vtxIdx = gl_VertexIndex;
    } 
    /* Line */
    else if (drawData.primType == 1) {
    /* Start by turning two seperate triangles into a quad by assigning a vertex index*/
        lineStart = gl_BaseInstanceARB + ((gl_InstanceIndex - gl_BaseInstanceARB)*2);
        lineEnd = lineStart + 1;
        vtxIdx = lineIdx[gl_VertexIndex] == 0 ? lineEnd : lineStart;
    }
    /* Points */
    else if (drawData.primType == 2) {
        vtxIdx = gl_InstanceIndex;
    }

    /* Load Vertex */
    sVertexData vertexData = vertexBuffers[drawData.vertBuffBindIdx].data[vtxIdx];
    sVertexData lineStartData; 
    sVertexData lineEndData;
    if (drawData.primType == 1) {
        lineStartData = vertexBuffers[drawData.vertBuffBindIdx].data[lineStart];
        lineEndData = vertexBuffers[drawData.vertBuffBindIdx].data[lineEnd];
    }

    vec3 inPosition = vertexData.pos;
    float inSize = vertexData.scale;
    vec4 inColor = unpackUnorm4x8(vertexData.icolor).abgr;

    /* Triangles */
    vec4 outPosition;
    if(drawData.primType == 0) {
        outPosition = drawData.viewProj * vec4(inPosition, 1.0);
    }
    /* Line expansion */
    if (drawData.primType == 1) {
        vec4 posStart = drawData.viewProj * vec4(lineStartData.pos, 1.0);
        vec4 posEnd = drawData.viewProj * vec4(lineEndData.pos, 1.0);
        vec2 dir = (posStart.xy / posStart.w) - (posEnd.xy / posEnd.w);
        dir = normalize(vec2(dir.x, dir.y * drawData.viewSize.y / drawData.viewSize.x));
        vec2 tng = vec2(-dir.y, dir.x) * 4 / drawData.viewSize;
        outPosition = lineIdx[gl_VertexIndex] == 0 ? posStart : posEnd;
		outPosition.xy += quadVerts[gl_VertexIndex] * tng * outPosition.w;
    }
    /* Point expansion */
    if (drawData.primType == 2) {
        outPosition = drawData.viewProj * vec4(inPosition, 1.0);
        vec2 scale = 1.0 / drawData.viewSize * vertexData.scale;
        outPosition.xy += quadVerts[gl_VertexIndex] * scale * outPosition.w;
    }
    outColor = inColor;
    gl_Position = outPosition;
}
#endif

#ifdef FRAGMENT_SHADER
layout(location = 0) in vec4 inColor;
layout(location = 0) out vec4 outColor;

void main() {
    outColor = inColor;
}
#endif