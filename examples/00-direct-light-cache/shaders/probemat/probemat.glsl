#ifndef PROBEMAT_GLSL
#define PROBEMAT_GLSL

#define PROBEMATPAYLOAD_OUT
#include "payload.glsl"

vec3 performProbeMat(vec3 origin, float tmin, vec3 dir, float len, ProbeMatTraceConfig config, uint seed) {
	ProbeMatOutPayload = constructProbeMatPayload(config, seed);

	traceRayEXT(
	MainTlas, // Top Level Acceleration Structure
	0,
	0xff, // Culling Mask (Possible use: Skip intersection which don't have a specific bit set)
	PROBEMAT_GROUP_ID,
	0,
	PROBEMAT_GROUP_ID, // Miss Index (the visibility test miss shader)
	origin, // Ray origin in world space
	tmin, // Minimum ray travel distance
	dir, // Ray direction in world space
	len, // Maximum ray travel distance
	4// Payload index (outgoing payload bound to location 0 in payload.glsl)
	);
	return ProbeMatOutPayload.hit.Radiance;
}

#endif
