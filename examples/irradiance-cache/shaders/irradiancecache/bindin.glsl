#include "irradiancecache.glsl"

#ifndef BIND_IN_IRRADIANCE_CACHE
#error "BIND_IN_IRRADIANCE_CACHE not set!"
#endif

#ifndef SET_IN_IRRADIANCE_CACHE
#define SET_IN_IRRADIANCE_CACHE 0
#endif

layout(set = SET_IN_IRRADIANCE_CACHE, binding = BIND_IN_IRRADIANCE_CACHE, rgba32f) uniform image3D IrradianceCacheIn;

/// read
vec4 readIrradianceCache(uvec3 pixelSpace) {
	return imageLoad(IrradianceCacheIn, ivec3(pixelSpace));
}

vec4 sampleIrradianceCache(vec3 worldSpace, vec3 normal) {
	const float normalOffsetFactor = irradianceCacheNormalOffsetFactor();
	vec3 normalOffset = normal * normalOffsetFactor;

	vec3 voxelGlobal = transformWorldToIrradiancePixel(worldSpace) + normalOffset;
	uvec3 voxel = uvec3(voxelGlobal);
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
	for (uint z = 0; z < 2; z++) {
		for (uint y = 0; y < 2; y++) {
			for (uint x = 0; x < 2; x++) {
				probe[z*4+y*2+x] = probeWeight[z*4+y*2+x] > 0.001 ? readIrradianceCache(voxel + uvec3(x, y, z)) : vec4(0);
			}
		}
	}

	// ignore dark texels, disabled in direct_ic mode
	if (irradianceCacheMode() != MODE_DIRECT_IC) {
		for (uint i = 0; i < 8; i++) {
			const float compeltelyDarkTexelsLimit = 0.015;
			if ((abs(probe[i].x) + abs(probe[i].y) + abs(probe[i].z)) < compeltelyDarkTexelsLimit) {
				probeWeight[i] = 0;
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

#undef BIND_IN_IRRADIANCE_CACHE
#undef SET_IN_IRRADIANCE_CACHE
