#ifndef PROBEMATPAYLOAD_GLSL
#define PROBEMATPAYLOAD_GLSL

#include "rt_common/payload.glsl"
#include "visitest/visitest.glsl"

struct ProbeMatTraceConfig {
	bool sampleIrradianceCache;
	bool traceDirectLight;
	uint traceIndirectLightDepth;
	uint perfectlyReflectiveDepth;
};

struct ProbeMatPayload
{
	HitPayload hit;
	ProbeMatTraceConfig config;
};

#endif

#ifdef PROBEMATPAYLOAD_OUT
layout(location = 4) rayPayloadEXT ProbeMatPayload ProbeMatOutPayload;
#undef PROBEMATPAYLOAD_OUT
#endif
#ifdef PROBEMATPAYLOAD_IN
layout(location = 5) rayPayloadInEXT ProbeMatPayload ProbeMatInPayload;
#undef PROBEMATPAYLOAD_IN
#endif
