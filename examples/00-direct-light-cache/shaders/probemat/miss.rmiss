#version 460
#extension GL_GOOGLE_include_directive : enable // Include files
#extension GL_EXT_ray_tracing : enable // Raytracing

#include "bindpoints.glsl"

#define PROBEMATPAYLOAD_IN
#include "payload.glsl"

void main()
{
    // The hit shader is invoked, when no geometry has been hit. We could render a skybox in here
    ProbeMatInPayload.hit.Radiance = vec3(0.0, 0.0, 0.0);
}
