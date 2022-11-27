# About
CitrusToolbox uses it's own portability layer to adapt between c-style shading languages

# File Formats

* CitrusToolbox Shader Header: ".ctsh"
* CitrusToolbox Shader Implementation: ".ctsi"

# Designer Shaders

If you just want to create an artistic shader effect for materials, that is simple!

## Boilerplate

	#include "templates/material.ctsh"
   
## Material Contents

todo
   
## Vertex Manipulation

### ctVertexData Variables

	float3 position;
	float3 velocity;
	float3 normal;
	float4 color;
	float4 tangent;
	float4 bitangent;
	float2 uv0;
	float2 uv1;

### Function

	#define CT_DESIGNER_VERTEX_PROCESS
	void ctVertexProcess(inout ctVertexData vertex) {
		// todo
	}

## Surface Manipulation

### ctSurfaceData Variables

	float3 baseColor;
	float3 emission;
	float roughness;
	float metal;
	float opacity;
	float translucency;
	float3 normal;
	float3 velocity;
	
### Function

	#define CT_DESIGNER_SURFACE_PROCESS
	void ctSurfaceProcess(in ctVertexData vertex, inout ctSurfaceData surface) {
		// todo
	}
	
# Markup

### Depth Modification

If you write to 

## Compilation Info

Compilation data is stored alongside the shader inside a comment block, this compilation data is not subject to preprocessor blocks and will always be included if it is found in one of the possibly included files. The fx blocks are always appended.

	/*CT_COMPILE_INFO: {
		"fx":[
			{
				"name":"FX_NAME",
				"stages":["VERTEX_SHADER","FRAGMENT_SHADER"],
				"backends":["all"],
				"defines":{
					"MY_DEFINE_A":"0",
					"MY_DEFINE_B":"FOO()"
				}
			}
		]
	}*/
	
You can #include already defined compile types from the templates folder, you shouldn't need this for authoring material shaders.

# Includes
External file inclusions are can be expanded offline

    #include "foo.ctsh"

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
|CT_FRAG_POSITION|SV_Position|gl_FragCoord|
|CT_PRIMITIVE_ID|SV_PrimitiveID|gl_PrimitiveID|
|CT_SAMPLE_INDEX|SV_SampleIndex|gl_SampleID|
|CT_STENCIL_REF|SV_StencilRef|gl_FragStencilRef|
|CT_LAYER|SV_RenderTargetArrayIndex|gl_Layer|
|CT_VIEWPORT_INDEX|SV_ViewportArrayIndex|gl_ViewportIndex|
|CT_INSTANCE_INDEX|SV_InstanceID|gl_InstanceIndex|
|CT_VERTEX_INDEX|SV_VertexID|gl_VertexIndex|

# Math types

Follows a subset of HLSL

* Scalars: float, int, uint
* Vectors: float#, int#, uint#
* Matrices: float#x#

# Martices
|     Macro      |     Definition  |
|----------------|----------------|
|CT_MMULV(M,V)|Multiplies matrix with a vector|
|CT_MELM(M,R,C)|Accesses a element by row/column|
|CT_MCOL(M,T,N)|Gets a type of T from a matrix from column N|
|CT_MROW(M,T,N)|Gets a type of T from a matrix from row N|

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

## Storage/Structured Buffer Example

    struct sVertexData {
        ...
    };
    
    CT_STORAGE_BUFFER_ARRAY_IN(sVertexBuffer, vertexBuffers, {
        sVertexData data[]; // multiple elements
    })
	
	...
	
	// sampling
	vertexBuffers[bufferIdx].data[element]
	
## Padding

Worst case scenerio Vulkan padding is assumed: https://fvcaputo.github.io/2019/02/06/memory-alignment.html

## Uniform/Constant Buffers?

TBD

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

### Builtin Vertex Attributes

* CT_SET_POSITION(position)
* CT_SET_POINT_SIZE(size)

### Builtin Fragment Attributes

* CT_SET_FRAG_DEPTH(depth)

### Custom Attributes

Start by defining the appropriate CT_ATTRIBUTES

* CT_ATTRIBUTES_FRAG_IN
* CT_ATTRIBUTES_FRAG_OUT

Add #include "utilities/attributes.ctsh"
after each attribute macro switch

using the following macros to describe the structure

* CT_ATTRIBUTES_BEGIN()
* CT_ATTRIBUTES_VAR(TYPE,NAME)
* CT_ATTRIBUTES_END()

On the beginning of the main function you can use

* CT_GET_ATTRIBUTE(TYPE,NAME)
* CT_SET_ATTRIBUTE(NAME,VALUE)

**Important!** The "CT_ATTRIBUTES_XXX" structure must be accessible from the recieving stage

## Eample

	#define CT_ATTRIBUTES_FRAG_IN
	#include "utilities/attributes.ctsh"
	CT_ATTRIBUTES_BEGIN()
	CT_ATTRIBUTES_VAR(0,float4,color)
	CT_ATTRIBUTES_VAR(1,float2,uv)
	CT_ATTRIBUTES_VAR(2,float3,normal)
	CT_ATTRIBUTES_END()
	#undef CT_ATTRIBUTES_FRAG_IN
	
	#ifdef VERTEX_SHADER
	void main(){
		...
		
		CT_SET_ATTRIBUTE(color,float4(1.0));
		CT_SET_ATTRIBUTE(uv,float2(0.0));
		CT_SET_ATTRIBUTE(normal,float3(0.5));
	}
	#elif FRAGMENT_SHADER
	void main(){
		CT_GET_ATTRIBUTE(float4,color);
		CT_GET_ATTRIBUTE(float2,uv);
		CT_GET_ATTRIBUTE(float3,normal);
		
		...
	}
	#endif

# Vertex Buffer Attributes

Vertex buffer attributes due to legacy reasons (cough HLSL) are much more fixed in input type.
These are specifically to be used in the vertex shader stage. 
In ctGPU the order you define these as determines in what order your bindings are.

Start with CT_DEFINE_VBUF_BEGIN() and end with CT_DEFINE_VBUF_END()

## Definition

* CT_DEFINE_VBUF_POSITION
* CT_DEFINE_VBUF_NORMAL
* CT_DEFINE_VBUF_TANGENT
* CT_DEFINE_VBUF_COLOR(0-3)
* CT_DEFINE_VBUF_TEXCOORD(0-6)
* CT_DEFINE_VBUF_SKIN_INDICES
* CT_DEFINE_VBUF_SKIN_WEIGHTS

## Sampling

* CT_VBUF_POSITION
* CT_VBUF_NORMAL
* CT_VBUF_TANGENT
* CT_VBUF_COLOR(0-3)
* CT_VBUF_TEXCOORD(0-6)
* CT_VBUF_SKIN_INDICES
* CT_VBUF_SKIN_WEIGHTS

## Example 

	#ifdef VERTEX_SHADER

	CT_DEFINE_VBUF_BEGIN()
	CT_DEFINE_VBUF_POSITION()
	CT_DEFINE_VBUF_NORMAL()
	CT_DEFINE_VBUF_TEXCOORD3()
	...
	CT_DEFINE_VBUF_END()

	void main() {
		float3 position = CT_GET_VBUF_POSITION().xyz
		...
	}

	#endif

# Entry Functions

Always CT_MAIN

# Resource Binding
Bindless will be used and provided per-language/backend

BINDLESS_IDX is used as the data type for a resource binding

# Other Features
* Pretty much everything math related from HLSL will be #defined in GLSL and other backends
* Coordinate spaces will always be flipped by the macro to match Vulkan
* To set a point size in compatible languages use CT_SET_POINT_SIZE(size)

# Credit
Thanks to https://anteru.net/blog/2016/mapping-between-HLSL-and-GLSL/