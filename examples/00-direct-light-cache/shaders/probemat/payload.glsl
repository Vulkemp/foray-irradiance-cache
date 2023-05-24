#ifndef PROBEMATPAYLOAD_GLSL
#define PROBEMATPAYLOAD_GLSL

#include "rt_common/payload.glsl"
#include "visitest/visitest.glsl"

struct ProbeMatTraceConfig {
	bool sampleIrradianceCache;
	bool traceDirectLight;
};

struct ProbeMatPayload
{
	HitPayload hit;
	ProbeMatTraceConfig config;
};

ProbeMatPayload constructProbeMatPayload(ProbeMatTraceConfig config, uint seed)
{
	ProbeMatPayload probe = { ConstructHitPayload(), config };
	probe.hit.Seed = seed;
	return probe;
}

#ifdef PROBEMATPAYLOAD_OUT
layout(location = 4) rayPayloadEXT ProbeMatPayload ProbeMatOutPayload;
#endif
#ifdef PROBEMATPAYLOAD_IN
layout(location = 5) rayPayloadInEXT ProbeMatPayload ProbeMatInPayload;
#endif

#endif
