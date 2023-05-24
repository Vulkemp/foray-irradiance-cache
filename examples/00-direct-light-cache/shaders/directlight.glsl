#include "rt_common/simplifiedlights.glsl"
#include "visitest/visitest.glsl"

// Do a maximum of 5 light tests (since each is a ray cast, which is quite expensive)
const uint DIRECT_LIGHT_TEST_CNT = 5;

vec3 CollectDirectLight(const VisiTestConfig visiTestConfig, vec3 pos, uint seed)
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
		if (!performVisiTest(visiTestConfig, origin, 0.001, dir, len))
		{
			directLightSum += (light.Intensity * light.Color) / (4 * PI) * dropoffFactor;
			directLightWeight += 1;
		}
	}

	return directLightWeight > 0 ? directLightSum / directLightWeight : vec3(0);
}
