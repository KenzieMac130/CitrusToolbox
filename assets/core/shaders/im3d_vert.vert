
const vec3 positions[] = {
    vec3(0.0, 1.0, 0.0),
    vec3(1.0, -1.0, 0.0),
    vec3(-1.0, -1.0, 0.0)
};

const vec4 colors[] = {
    vec4(1.0, 0.0, 0.0, 1.0),
    vec4(0.0, 1.0, 0.0, 1.0),
    vec4(0.0, 0.0, 1.0, 1.0)
};

layout(push_constant) uniform sPushConstant {
    mat4 viewProj;
    uint vertBuffBindIdx;
    uint primType;
} drawData;

layout(location = 0) out vec4 outColor;

void main(){
    outColor = colors[gl_VertexIndex];
    gl_Position = drawData.viewProj * vec4(positions[gl_VertexIndex], 1.0);
}