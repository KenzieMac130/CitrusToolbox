# About
CitrusToolbox uses an augmented version of glsl to make porting easier

# Includes
External file inclusions are added to glsl, this can be expanded offline

    #include "foo.glsl"

# Builtin Values
|     Macro      |     HLSL       |     GLSL       |
|----------------|----------------|----------------|
|CT_CLIP_DISTANCE|SV_ClipDistance|gl_ClipDistance|
|CT_CULL_DISTANCE|SV_CullDistance|gl_CullDistance|
|CT_COVERAGE_IN|SV_Coverage|gl_SampleMaskIn|
|CT_COVERAGE_OUT|SV_Coverage|gl_SampleMask|
|CT_DEPTH|SV_Depth|gl_FragDepth|
|CT_DEPTH_OUT_LESS_EQUAL|SV_DepthLessEqual|(MACRO EXPANSION)|
|CT_DEPTH_OUT_GREATER_EQUAL|SV_DepthGreaterEqual|(MACRO EXPANSION)|
|CT_DISPATCH_TREAD_ID|SV_DispatchThreadID|gl_GlobalInvocationID|
|CT_GROUP_ID|SV_GroupID|gl_WorkGroupID|
|CT_GROUP_INDEX|SV_GroupIndex|gl_LocalInvocationIndex|
|CT_GROUP_THREAD_ID|SV_GroupThreadID|gl_LocalInvocationID|
|CT_GS_INSTANCE_ID|SV_GSInstanceID|gl_InvocationID|
|CT_TESS_LOCATION|SV_DomainLocation|gl_TessCord|
|CT_TESS_LEVEL_INNER|SV_InsideTessFactor|gl_TessLevelInner|
|CT_TESS_LEVEL_OUTER|SV_TessFactor|gl_TessLevelOuter|
|CT_IS_FRONT_FACE|SV_IsFrontFace|gl_FrontFacing|
|CT_OUTPUT_CONTROL_POINT_ID|SV_OutputControlPointID|gl_InvocationID|
|CT_VERT_POSITION|SV_Position|gl_Position|
|CT_FRAG_POSITION|SV_Position|gl_FragCoord|
|CT_PRIMITIVE_ID|SV_PrimitiveID|gl_PrimitiveID|
|CT_SAMPLE_INDEX|SV_SampleIndex|gl_SampleID|
|CT_STENCIL_REF|SV_StencilRef|gl_FragStencilRef|
|CT_LAYER|SV_RenderTargetArrayIndex|gl_Layer|
|CT_RENDER_TARGET(idx)|SV_Target|layout(location=N) out|
|CT_VIEWPORT_INDEX|SV_ViewportArrayIndex|gl_ViewportIndex|
|CT_INSTANCE_INDEX|SV_InstanceID|gl_InstanceIndex|
|CT_VERTEX_INDEX|SV_VertexID|gl_VertexIndex|

# Texturing

## Texture1D Example
    CT_TEXTURE1D(S_FILTERED, textureIdx, u)
    CT_TEXTURE1D_LOD(S_FILTERED, textureIdx, u, level)

## Texture2D Example
    CT_TEXTURE2D(S_FILTERED, textureIdx, uv)
	CT_TEXTURE2D_LOD(S_FILTERED, textureIdx, uv, level)
	
## Texture3D Example
	CT_TEXTURE3D(S_FILTERED, textureIdx, uvw)
	CT_TEXTURE3D_LOD(S_FILTERED, textureIdx, uvw, level)
	
## TextureCube Example
	CT_TEXTURECUBE(S_FILTERED, textureIdx, direction)
	CT_TEXTURECUBE_LOD(S_FILTERED, textureIdx, direction, level)
	
## Texture2D Depth Example
    CT_TEXTURE2D_DEPTH(S_DEPTH_BEHIND_FILTERED, textureIdx, uv)	
    CT_TEXTURE2D_DEPTH_LOD(S_DEPTH_BEHIND_FILTERED, textureIdx, uv, level)	
	
## TextureCube Depth Example
	CT_TEXTURECUBE_DEPTH(S_DEPTH_INFRONT_FILTERED, textureIdx, direction)
	CT_TEXTURECUBE_DEPTH_LOD(S_DEPTH_INFRONT_FILTERED, textureIdx, direction, level)

# Samplers

## Generic Samplers
* S_FILTERED
* S_FILTERED_NO_ANISO
* S_NEAREST

## Depth Samplers
* S_DEPTH_BEHIND_FILTERED
* S_DEPTH_INFRONT_FILTERED
* S_DEPTH_EQUAL_FILTERED
* S_DEPTH_BEHIND_FILTERED_NO_ANISO
* S_DEPTH_INFRONT_FILTERED_NO_ANISO
* S_DEPTH_EQUAL_FILTERED_NO_ANISO
* S_DEPTH_BEHIND_NEAREST
* S_DEPTH_INFRONT_NEAREST
* S_DEPTH_EQUAL_NEAREST

# Buffers

* CT_CONSTANT_BUFFER(sStructName, variableName, \{ ... \})
* CT_STORAGE_BUFFER_IN(sStructName, variableName, \{ ... \})
* CT_STORAGE_BUFFER_OUT(sStructName, variableName, \{ ... \})
* CT_STORAGE_BUFFER_INOUT(sStructName, variableName, \{ ... \})

## Constant Buffer Example
    
	// each shader can have their own structure
    CT_CONSTANT_BUFFER(sMaterialBuffer, materials, {
        vec4 color;
        float roughness;
        float metal;
        BINDLESS_IDX baseTexture;
        BINDLESS_IDX pbrTexture;
        BINDLESS_IDX normalTexture;
    })
	
	...
	
	// sampling
	materials[materialIdx]

## Structured Buffer Example

    struct sVertexData {
        ...
    };
    
    CT_STORAGE_BUFFER_ARRAY_IN(sVertexBuffer, vertexBuffers, {
        sVertexData data[]; // multiple elements
    })
	
	...
	
	// sampling
	vertexBuffers[bufferIdx].data[element]
	
# Outputs

# Stages

The following macros will be defined based on stage

* VERTEX_SHADER
* TESSELLATION_CONTROL_SHADER
* TESSELLATION_EVALUATION_SHADER
* GEOMETRY_SHADER
* FRAGMENT_SHADER
* COMPUTE_SHADER
* RAY_GENERATION_SHADER
* RAY_ANY_HIT_SHADER
* RAY_CLOSEST_HIT_SHADER
* RAY_INTERSECTION_SHADER
* CALLABLE_SHADER
* TASK_SHADER
* MESH_SHADER

# Stage Attributes

## Vertex Inputs (LEGACY)

* CT_VERT_ATTRIBUTES_BEGIN()
* CT_VERT_ATTRIBUTES_POSITION(NAME)
* CT_VERT_ATTRIBUTES_NORMAL(NAME)
* CT_VERT_ATTRIBUTES_TANGENT(NAME)
* CT_VERT_ATTRIBUTES_UV(NAME, SLOT)
* CT_VERT_ATTRIBUTES_COLOR(NAME, SLOT)
* CT_VERT_ATTRIBUTES_BONE_IDX(NAME)
* CT_VERT_ATTRIBUTES_BONE_WEIGHT(NAME)
* CT_VERT_ATTRIBUTES_END()

## Interpolation

* CT_INTERPOLATE_ATTRIBUTES_BEGIN()
* CT_INTERPOLATE_ATTRIBUTES_VAR(TYPE,NAME)
* CT_INTERPOLATE_ATTRIBUTES_END()

## Fragment Outputs

* CT_FRAGMENT_ATTRIBUTES_BEGIN()
* CT_FRAGMENT_ATTRIBUTES_VAR(TYPE,NAME)
* CT_FRAGMENT_ATTRIBUTES_END()

# Entry Functions

# Input Attachments

> Input attachment grabbing is currently suspended due to the use of VK_KHR_dynamic_rendering

This feature is difficult to emulate but will be emulated via texture fetches fed by
the render architect depending on the backend if not avalible. This will require wrapping 
the subpass input definitions like so:

    #define CT_SUBPASS_INPUTS\
    CT_SUBPASS_INPUT(INDEX,BINDING,NAME)\
    CT_SUBPASS_INPUT(INDEX,BINDING,NAME)
	
And place the following at the beginning of the fragment shader

    CT_SUBPASS_INPUT_BOILERPLATE()

# Resource Binding
Bindless will be used and provided per-language/backend

BINDLESS_IDX is used as the data type for a resource binding

# Compute
Todo

# Other Features
* Pretty much everything math related from GLSL will be #defined in HLSL
* saturate() will be added to GLSL
* Coordinate spaces will always be flipped by the macro to match Vulkan
* Matrix * Vector multiplication will be done with MATRIX_MULTIPLY(M,V)

# Credit
Thanks to https://anteru.net/blog/2016/mapping-between-HLSL-and-GLSL/