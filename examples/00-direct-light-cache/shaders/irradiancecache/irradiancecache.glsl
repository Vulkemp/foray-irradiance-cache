#ifndef IRRADIANCECACHE_GLSL
#define IRRADIANCECACHE_GLSL

#ifdef PUSH_CONSTANTS_DEFINED
#error "Push constants were already defined!"
#endif
#define PUSH_CONSTANTS_DEFINED

struct IrradianceCacheConfig {
// w unused
	vec4 origin;
// w unused
	vec4 extent;
// w unused
	vec4 imageExtent;
};

layout(push_constant) uniform TracerConfigBlock
{
/// @brief Per frame unique seed for random number generation
	IrradianceCacheConfig irrConfig;
	uint RngSeed;
}
TracerConfig;

vec3 transformWorldToIrradianceCache(vec3 v) {
	IrradianceCacheConfig c = TracerConfig.irrConfig;
	return (v - c.origin.xyz) / c.extent.xyz;
}

vec3 transformIrradianceCacheToWorld(vec3 v) {
	IrradianceCacheConfig c = TracerConfig.irrConfig;
	return v * c.extent.xyz + c.origin.xyz;
}

vec3 transformIrradiancePixelToIrradianceCache(uvec3 v) {
	IrradianceCacheConfig c = TracerConfig.irrConfig;
	return (vec3(v) + vec3(0.5)) / c.imageExtent.xyz;
}

vec3 transformIrradiancePixelToWorldSpace(uvec3 v) {
	return transformIrradianceCacheToWorld(transformIrradiancePixelToIrradianceCache(v));
}



#ifdef BIND_OUT_IRRADIANCE_CACHE
#ifndef SET_OUT_IRRADIANCE_CACHE
#define SET_OUT_IRRADIANCE_CACHE 0
#endif

layout(set = SET_OUT_IRRADIANCE_CACHE, binding = BIND_OUT_IRRADIANCE_CACHE, rgba32f) uniform writeonly image3D IrradianceCache;

void writeIrradianceCache(uvec3 pixelSpace, vec4 value) {
	imageStore(IrradianceCache, ivec3(pixelSpace), value);
}
#endif

#ifdef BIND_IN_IRRADIANCE_CACHE
#ifndef SET_IN_IRRADIANCE_CACHE
#define SET_IN_IRRADIANCE_CACHE 0
#endif

layout(set = SET_IN_IRRADIANCE_CACHE, binding = BIND_IN_IRRADIANCE_CACHE) uniform sampler3D IrradianceCache;

vec4 readIrradianceCacheWorldSpace(vec3 worldSpace, vec3 normal) {
	return texture(IrradianceCache, transformWorldToIrradianceCache(worldSpace) + (normal * 1.1f) / TracerConfig.irrConfig.imageExtent.xyz);
}
#endif

#endif
