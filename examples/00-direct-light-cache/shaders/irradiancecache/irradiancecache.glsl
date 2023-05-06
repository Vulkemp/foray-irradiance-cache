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

vec4 sampleIrradianceCacheMix(bool beforeNormal[2], vec4 data[2], float t) {
//	if (!beforeNormal[0] && !beforeNormal[1]) {
//		// don't care: result will be ignored later
//	} else
	if (!beforeNormal[0]) {
		return data[1];
	} else if (!beforeNormal[1]) {
		return data[0];
	} else {
		return mix(data[0], data[1], t);
	}
}

vec4 sampleIrradianceCache(vec3 worldSpace, vec3 normal) {
	vec3 voxelGlobal = transformWorldToIrradiancePixel(worldSpace);
	ivec3 voxel = ivec3(voxelGlobal);
	vec3 voxelFrac = fract(voxelGlobal);

	// fetch texels
	// access: z*4+y*2+x
	vec4 texels[8];
	for (int z = 0; z < 2; z++) {
		for (int y = 0; y < 2; y++) {
			for (int x = 0; x < 2; x++) {
				texels[z*4+y*2+x] = texelFetch(IrradianceCache, voxel + ivec3(x, y, z), 0);
			}
		}
	}

	// true if coord is before normal plane, will be used multiple times
	// access: z*4+y*2+x
	bool beforeNormal[8];
	for (int z = 0; z < 2; z++) {
		for (int y = 0; y < 2; y++) {
			for (int x = 0; x < 2; x++) {
				beforeNormal[z*4+y*2+x] = dot(vec3(x, y, z) - voxelFrac, normal) > -0.01;
			}
		}
	}

	// interpolate bilinearly but only if texel is before normal plane
	vec4 result;
	bool anyBeforeNormal;
	{
		bool beforez[2];
		vec4 resultz[2];
		for (int z = 0; z < 2; z++) {
			bool beforey[2];
			vec4 resulty[2];
			for (int y = 0; y < 2; y++) {
				uint i = z*4+y*2;
				vec4 resultx[2] = { texels[i], texels[i+1] };
				bool beforex[2] = { beforeNormal[i], beforeNormal[i+1] };
				resulty[y] = sampleIrradianceCacheMix(beforex, resultx, voxelFrac.x);
				beforey[y] = beforex[0] || beforex[1];
			}
			resultz[z] = sampleIrradianceCacheMix(beforey, resulty, voxelFrac.y);
			beforez[z] = beforey[0] || beforey[1];
		}
		anyBeforeNormal = beforez[0] || beforez[1];
		result = sampleIrradianceCacheMix(beforez, resultz, voxelFrac.z);
	}

	// no voxels have been sampled -> black
	// TODO may not be required?
	if (!anyBeforeNormal) {
		result = vec4(0, 0, 0, 0);
	}

	return result;
//		return vec4(voxelFrac, 1);
	//	return vec4(beforeNormal[2] ? 1 : 0);
}
#endif

#endif
