#ifndef DIRECTLIGHT_GLSL
#define DIRECTLIGHT_GLSL

#include "rt_common/simplifiedlights.glsl"
#include "visitest/visitest.glsl"
#include "common/lcrng.glsl"
#include "common/gltf.glsl"
#include "common/materialbuffer.glsl"
#include "shading/sampling.glsl"
#include "shading/material.glsl"

// Do a maximum of 5 light tests (since each is a ray cast, which is quite expensive)
const uint DIRECT_LIGHT_TEST_CNT = 5;

vec3 collectDirectLightBase(vec3 pos, bool useMaterial, vec3 normal, vec3 wOut, MaterialBufferObject material, MaterialProbe probe, inout uint seed)
{
	const uint lightTestCount = min(DIRECT_LIGHT_TEST_CNT, SimplifiedLights.Count);

	vec3 directLightSum = vec3(0);
	int directLightWeight = 0;

	for (uint i = 0; i < lightTestCount; i++)
	{
		// Randomly select a light source
		SimplifiedLight light = SimplifiedLights.Array[lcgUint(seed) % SimplifiedLights.Count];

		// light properties
		vec3 origin = pos;
		vec3 dir = vec3(0);
		float len = 0;
		float dropoffFactor = 1;
		if (light.Type == SimplifiedLightType_Directional)
		{
			dir = normalize(light.PosOrDir);
			len = INFINITY;
		}
		else
		{
			dir = light.PosOrDir - origin;
			len = length(dir);
			dir = normalize(dir);
			dropoffFactor = 1 / (len * len);
		}

		// If light source is visible ...
		if (!performVisiTest(origin, 0.001, dir, len))
		{
			vec3 light = (light.Intensity * light.Color) / (4 * PI) * dropoffFactor;

			if (useMaterial) {
				HitSample hit;
				hit.Normal = normal;
				hit.wOut = wOut;
				hit.wIn = dir;
				hit.wHalf = normalize(hit.wOut + hit.wIn);
				light *= EvaluateMaterial(hit, material, probe);
			}

			directLightSum += light;
			directLightWeight += 1;
		}
	}

	return directLightWeight > 0 ? directLightSum / directLightWeight : vec3(0);
}

vec3 collectDirectLight(vec3 pos, inout uint seed) {
	MaterialBufferObject material = GetMaterialOrFallback(-1);
	MaterialProbe probe = ProbeMaterial(material, vec2(0.5, 0.5));
	return collectDirectLightBase(pos, false, vec3(0), vec3(0), material, probe, seed);
}

vec3 collectDirectLightAndMaterial(vec3 pos, vec3 normal, vec3 wOut, MaterialBufferObject material, MaterialProbe probe, vec3 attenuation, inout uint seed) {
	return collectDirectLightBase(pos, true, normal, wOut, material, probe, seed) * attenuation;
}

#endif
