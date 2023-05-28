#include "common/lcrng.glsl"
#include "common/gltf.glsl"
#include "common/materialbuffer.glsl"
#include "shading/sampling.glsl"
#include "shading/material.glsl"
#include "probemat/probemat.glsl"

// Offsets a ray origin slightly away from the surface to prevent self shadowing
void CorrectOrigin(inout vec3 origin, vec3 normal, float nDotL)
{
	float correctorLength = clamp((1.0 - nDotL) * 0.005, 0, 1);
	origin += normal * correctorLength;
}

vec3 collectIndirectLightAndMaterial(vec3 pos, vec3 normal, vec3 wOut, MaterialBufferObject material, MaterialProbe probe, vec3 attenuation, inout uint seed, ProbeMatTraceConfig config)
{
	vec3 sumIndirect = vec3(0);
	int weightIndirect = 0;

	// Calculate count of secondary rays to emit. Use current rays Attenuation for bailout
	const float attenuationModifier = min(dot(attenuation, attenuation), 1);
	const float rng = lcgFloat(seed);
	const float modifier = 1.0;
	uint secondary = uint(max(0, attenuationModifier + attenuationModifier * rng * modifier));

	bool perfectlyReflective = probe.MetallicRoughness.r > 0.99 && probe.MetallicRoughness.g < 0.01;
	if (perfectlyReflective)
	{
		// Use at most 1 ray for perfectly reflective surfaces
		secondary = min(secondary, 1);
	}

	for (uint i = 0; i < secondary; i++)
	{
		seed += 1;
		float alpha = probe.MetallicRoughness.g * probe.MetallicRoughness.g;
		vec3 origin = pos;

		HitSample hit;
		hit.wOut = wOut;
		hit.Normal = perfectlyReflective ? normal : importanceSample_GGX(seed, probe.MetallicRoughness.g, normal);
		hit.wIn = normalize(-reflect(hit.wOut, hit.Normal));
		hit.wHalf = normalize(hit.wOut + hit.wIn);

		float ndotl = dot(hit.wIn, hit.Normal);
		float ndotv = dot(hit.wOut, hit.Normal);
		CorrectOrigin(origin, normal, ndotl);

		// If expected contribution is high enough ...
		vec3 newAttenuation = EvaluateMaterial(hit, material, probe);
		if (dot(newAttenuation, newAttenuation) > 0.001)
		{
			ProbeMatTraceConfig newConfig;
			newConfig.sampleIrradianceCache = config.sampleIrradianceCache;
			newConfig.traceDirectLight = config.traceDirectLight;
			newConfig.traceIndirectLightDepth = config.traceIndirectLightDepth - 1;

			sumIndirect += performProbeMat(origin, 0.001, hit.wIn, INFINITY, newConfig, seed + i, newAttenuation);
			weightIndirect += 1;
		}
	}
	return weightIndirect > 0 ? sumIndirect / weightIndirect : vec3(0);
}
