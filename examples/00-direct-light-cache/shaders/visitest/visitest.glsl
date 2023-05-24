#ifndef VISITEST_GLSL
#define VISITEST_GLSL

#define VISIPAYLOAD_OUT
#include "payload.glsl"

bool performVisiTest(vec3 origin, float tmin, vec3 dir, float len) {
	// Perform visibility test
	VisiPayload.hit = true;

	traceRayEXT(MainTlas, // Top Level Acceleration Structure
		gl_RayFlagsTerminateOnFirstHitEXT | gl_RayFlagsSkipClosestHitShaderEXT, // All we care about is the miss shader to tell us the lightsource is visible
		0xff, // Culling Mask (Possible use: Skip intersection which don't have a specific bit set)
		VISITEST_GROUP_ID,
		0,
		VISITEST_GROUP_ID, // Miss Index (the visibility test miss shader)
		origin, // Ray origin in world space
		tmin, // Minimum ray travel distance
		dir, // Ray direction in world space
		len, // Maximum ray travel distance
		2 // Payload index (outgoing payload bound to location 0 in payload.glsl)
	);
	return VisiPayload.hit;
}

#endif
