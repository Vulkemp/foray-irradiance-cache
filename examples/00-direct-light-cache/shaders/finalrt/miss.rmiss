#version 460
#extension GL_GOOGLE_include_directive : enable // Include files
#extension GL_EXT_ray_tracing : enable // Raytracing

#define HITPAYLOAD_IN
#include "rt_common/payload.glsl"

void main()
{
    // The hit shader is invoked, when no geometry has been hit. We could render a skybox in here
    ReturnPayload.Radiance = vec3(0.0, 0.0, 0.0);
}
