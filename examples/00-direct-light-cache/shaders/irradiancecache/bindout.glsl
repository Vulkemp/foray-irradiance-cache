#include "irradiancecache.glsl"

#ifndef BIND_OUT_IRRADIANCE_CACHE
#error "BIND_OUT_IRRADIANCE_CACHE not set!"
#endif

#ifndef SET_OUT_IRRADIANCE_CACHE
#define SET_OUT_IRRADIANCE_CACHE 0
#endif

layout(set = SET_OUT_IRRADIANCE_CACHE, binding = BIND_OUT_IRRADIANCE_CACHE, rgba32f) uniform writeonly image3D IrradianceCacheOut;

void writeIrradianceCache(uvec3 pixelSpace, vec4 value) {
	imageStore(IrradianceCacheOut, ivec3(pixelSpace), value);
}

#undef BIND_OUT_IRRADIANCE_CACHE
#undef SET_OUT_IRRADIANCE_CACHE
