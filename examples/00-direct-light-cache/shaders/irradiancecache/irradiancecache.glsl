#ifndef IRRADIANCECACHE_GLSL
#define IRRADIANCECACHE_GLSL


/// push constants
#ifdef PUSH_CONSTANTS_DEFINED
#error "Push constants were already defined!"
#endif
#define PUSH_CONSTANTS_DEFINED

const uint MODE_INDIRECT_TRACE_DIRECT_TRACE = 0;
const uint MODE_INDIRECT_IC_DIRECT_TRACE = 1;
const uint MODE_INDIRECT_IC = 2;
const uint MODE_DIRECT_TRACE = 3;
const uint MODE_DIRECT_IC = 4;
const uint MODE_DEBUG_PATTERN = 5;

struct IrradianceCacheConfig {
// w unused
	vec4 origin;
// w unused
	vec4 extent;
// w unused
	vec4 imageExtent;
// x IrradianceCacheMode
// y bool clearCache
// z indirect: traces per frame
// w indirect: accumulation factor
	uvec4 config;
};

layout(push_constant) uniform TracerConfigBlock
{
/// @brief Per frame unique seed for random number generation
	IrradianceCacheConfig irrConfig;
	uint RngSeed;
}
TracerConfig;


/// getter
uint irradianceCacheMode() {
	return TracerConfig.irrConfig.config.x;
}

bool irradianceCacheClearCache() {
	return TracerConfig.irrConfig.config.y != 0;
}

uint irradianceCacheIndirectTracesPerFrame() {
	return TracerConfig.irrConfig.config.z;
}

float irradianceCacheIndirectAccumulationFactor() {
	return uintBitsToFloat(TracerConfig.irrConfig.config.w);
}


/// transform
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

vec3 transformIrradianceCacheToIrradiancePixel(vec3 v) {
	IrradianceCacheConfig c = TracerConfig.irrConfig;
	return (v * c.imageExtent.xyz) - vec3(0.5);
}

vec3 transformIrradiancePixelToWorld(uvec3 v) {
	return transformIrradianceCacheToWorld(transformIrradiancePixelToIrradianceCache(v));
}

vec3 transformWorldToIrradiancePixel(vec3 v) {
	return transformIrradianceCacheToIrradiancePixel(transformWorldToIrradianceCache(v));
}

#endif
