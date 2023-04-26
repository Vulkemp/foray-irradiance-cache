#define VISIPAYLOAD_OUT
#include "payload.glsl"

struct VisiTestConfig {
	uint sbtRecordOffset;
	uint sbtRecordStride;
	uint missIndex;
};

bool performVisiTest(VisiTestConfig visiTest, vec3 origin, float Tmin, vec3 dir, float len) {
	// Perform visibility test
	VisiPayload.hit = true;

	traceRayEXT(MainTlas, // Top Level Acceleration Structure
		gl_RayFlagsTerminateOnFirstHitEXT | gl_RayFlagsSkipClosestHitShaderEXT, // All we care about is the miss shader to tell us the lightsource is visible
		0xff, // Culling Mask (Possible use: Skip intersection which don't have a specific bit set)
		visiTest.sbtRecordOffset,
		visiTest.sbtRecordStride,
		visiTest.missIndex, // Miss Index (the visibility test miss shader)
		origin, // Ray origin in world space
		0.001, // Minimum ray travel distance
		dir, // Ray direction in world space
		len, // Maximum ray travel distance
		2 // Payload index (outgoing payload bound to location 0 in payload.glsl)
	);
	return VisiPayload.hit;
}
