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

vec4 sampleIrradianceCache(vec3 worldSpace, vec3 normal) {
	const float normalOffsetFactor = 0.3;
	vec3 normalOffset = normal * normalOffsetFactor;

	vec3 voxelGlobal = transformWorldToIrradiancePixel(worldSpace + normalOffset);
	ivec3 voxel = ivec3(voxelGlobal);
	vec3 voxelFrac = fract(voxelGlobal);

	// weight of individual probes
	// access: z*4+y*2+x
	float probeWeight[8];
	for (uint i = 0; i < 8; i++) {
		probeWeight[i] = 1;
	}

	// probe weight *= based on before or after normal plane with soft threshold
	{
		const float softBias = 0.01;
		const float softThreshold = 0.01;
		for (int z = 0; z < 2; z++) {
			for (int y = 0; y < 2; y++) {
				for (int x = 0; x < 2; x++) {
					float distanceToPlane = dot(vec3(x, y, z) - voxelFrac + normalOffset, normal) + softBias;
					probeWeight[z*4+y*2+x] *= clamp(distanceToPlane / softThreshold, 0, 1);
				}
			}
		}
	}

	// fetch texels
	// access: z*4+y*2+x
	vec4 probe[8];
	for (int z = 0; z < 2; z++) {
		for (int y = 0; y < 2; y++) {
			for (int x = 0; x < 2; x++) {
				probe[z*4+y*2+x] = probeWeight[z*4+y*2+x] > 0.001 ? texelFetch(IrradianceCache, voxel + ivec3(x, y, z), 0) : vec4(0);
			}
		}
	}

	// probe weight *= voxelFrac
	for (int z = 0; z < 2; z++) {
		for (int y = 0; y < 2; y++) {
			for (int x = 0; x < 2; x++) {
				probeWeight[z*4+y*2+x] *=
				(x > 0 ? voxelFrac.x : 1-voxelFrac.x)
				* (y > 0 ? voxelFrac.y : 1-voxelFrac.y)
				* (z > 0 ? voxelFrac.z : 1-voxelFrac.z);
			}
		}
	}

	vec4 result = vec4(0);
	{
		float sumWeight = 0;
		for (uint i = 0; i < 8; i++) {
			result += probeWeight[i] * probe[i];
			sumWeight += probeWeight[i];
		}
		result = sumWeight > 0 ? (result / sumWeight) : vec4(0);
	}

	return result;
	//	return vec4(voxelFrac, 1);
}
#endif

#endif
