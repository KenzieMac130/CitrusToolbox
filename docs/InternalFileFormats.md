# Citrus Internal File Formats

## BNDL - Asset Bundle Manifest

### Description

Describes an asset and creates a catagorized indexable list of file dependencies to be associated with an asset. (ex. render mesh, collision, animation, texture list, etc)

### Format Basis
JSON Format

### Overview

```json
{
	"bundleType" : "...",
	"categories" : [
		{
			"type": "...",
			"files": [ "/example/data0.bin","/example/data1.bin" ]
		}
	]
}
```

## KLMD - KeyLime GPU 3D Model Content

### Description


### Format Basis
Binary Format

### Overview

```c
struct header {
	char magic[4] = { 'K', 'L', 'M', 'D' };
	uin32_t version; /* file version to avoid loading incompatible files */
};

/* Describes what texture to sample and how to map to it for a surface material */
struct surfaceMaterialTexture {
	int32_t textureIdx; /* Loads from offset in the bundle which is mapped into the bindless system */
	int32_t uvMap; /* Index into the model's UV maps */
	vec2 offset; /* Offset texture */
	vec2 scale; /* Scale texture */
	vec2 scroll; /* Offset texture with time */
};

/* Materials describe the properties to feed the shader per-mesh */
struct surfaceMaterial {
	vec4 baseColorAlpha; /* Base color and opacity */
	vec3 emissionColor; /* Emission color */
	int32_t surfaceProfile; /* Complex surface presets */
	
	float roughness; /* Perceptual Roughness */
	float translucentToMetal; /* 0.0: Translucent, 0.5: Dialectric, 1.0 Metal */
	float specular; /* Specular F0 */
	float clipLevel; /* Clip level for alpha clipping */
	
	float normalMapStrength; /* Normal mapping strength */
	float aoMapStrength; /* Ambient occlusion strength */
	float vertexColorBaseTint; /* Amount vertex color tints base color */
	float vertexColorEmissionTint; /* Amount vertex color tints emission color */
	
	/* Used for custom shaders */
	vec4 customVec0;
	vec4 customVec1;
	vec4 customVec2;
	vec4 customVec3;
	
	surfaceMaterialTexture baseTextureIdx; /* Base Texture and Alpha */ 
	surfaceMaterialTexture normalTextureIdx; /* Normalmap Texture (DirectX Format) */
	surfaceMaterialTexture pbrTextureIdx; /* PBR Texture (R: Occlusion, G: Metal/Translucent, B: Roughness) */
	surfaceMaterialTexture emissionTextureIdx; /* Emission Texture */
	
	/* Used for custom shaders */
	surfaceMaterialTexture customTex0;
	surfaceMaterialTexture customTex1;
	surfaceMaterialTexture customTex2;
	
	/* Used by app for debugging shaders per-material (don't save this) */
	int32_t debug = 0;
};

//todo: mesh section
struct mesh {
	int32_t materialIdx; /* Material */
	int32_t transformArrayIdx; /* Base and bone positions */
	
	int32_t indexArrayIdx; /* Geometry */
};

//todo: bones?

```

## CMDL - CPU 3D Model Content

### Description

### Format Basis
Binary Format

### Overview

```c
struct header {
	char magic[4] = { 'C', 'M', 'D', 'L' };
	uin32_t version; /* file version to avoid loading incompatible files */
};
```