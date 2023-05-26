#version 460
#extension GL_GOOGLE_include_directive : enable // Include files
#extension GL_EXT_ray_tracing : enable // Raytracing
#extension GL_EXT_nonuniform_qualifier : enable // Required for asserting that some array indexing is done with non-uniform indices

#include "bindpoints.glsl"
#include "common/materialbuffer.glsl"// Material buffer for material information and texture array
#include "rt_common/geometrymetabuffer.glsl"// GeometryMeta information
#include "rt_common/geobuffers.glsl"// Vertex and index buffer aswell as accessor methods
#include "common/normaltbn.glsl"// Normal calculation in tangent space
#include "common/lcrng.glsl"
#include "irradiancecache/bindin.glsl"
#include "visitest/visitest.glsl"
#include "shading/constants.glsl"
#include "shading/sampling.glsl"
#include "shading/material.glsl"
#include "directlight.glsl"

#define PROBEMATPAYLOAD_IN
#include "payload.glsl"

hitAttributeEXT vec2 attribs;// Barycentric coordinates

// Offsets a ray origin slightly away from the surface to prevent self shadowing
void CorrectOrigin(inout vec3 origin, vec3 normal, float nDotL)
{
	float correctorLength = clamp((1.0 - nDotL) * 0.005, 0, 1);
	origin += normal * correctorLength;
}

vec3 CollectLight(vec3 pos, vec3 normal, MaterialBufferObject material, MaterialProbe probe)
{
	// Irradiance Cache does not yet support incoming direction, so specular might look weird
	vec3 dir = normal;

	HitSample hit;
	hit.Normal = normal;
	hit.wOut = -gl_WorldRayDirectionEXT;
	hit.wIn = dir;
	hit.wHalf = normalize(hit.wOut + hit.wIn);

	// Calculate light reflected
	vec3 light = vec3(0);
	if (ProbeMatInPayload.config.sampleIrradianceCache) {
		light += sampleIrradianceCache(pos, normal).xyz;
	}
	if (ProbeMatInPayload.config.traceDirectLight) {
		light += collectDirectLight(pos, ProbeMatInPayload.hit.Seed);
	}

	vec3 reflection = light * EvaluateMaterial(hit, material, probe);
	return reflection * ProbeMatInPayload.hit.Attenuation;
}

void main()
{
	// The closesthit shader is invoked with hit information on the geometry intersect closest to the ray origin

	// STEP #1 Get meta information on the intersected geometry (material) and the vertex information

	// Get geometry meta info
	GeometryMeta geometa = GetGeometryMeta(uint(gl_InstanceCustomIndexEXT), uint(gl_GeometryIndexEXT));
	MaterialBufferObject material = GetMaterialOrFallback(geometa.MaterialIndex);

	// get primitive indices
	const uvec3 indices = GetIndices(geometa, uint(gl_PrimitiveID));

	// get primitive vertices
	Vertex v0, v1, v2;
	GetVertices(indices, v0, v1, v2);

	// STEP #2 Calculate UV coordinates and probe the material

	// Calculate barycentric coords from hitAttribute values
	const vec3 barycentricCoords = vec3(1.0f - attribs.x - attribs.y, attribs.x, attribs.y);

	// calculate uv
	const vec2 uv = v0.Uv * barycentricCoords.x + v1.Uv * barycentricCoords.y + v2.Uv * barycentricCoords.z;

	// Get material information at the current hitpoint
	MaterialProbe probe = ProbeMaterial(material, uv);

	// Calculate model and worldspace positions
	const vec3 posModelSpace = v0.Pos * barycentricCoords.x + v1.Pos * barycentricCoords.y + v2.Pos * barycentricCoords.z;
	const vec3 posWorldSpace = vec3(gl_ObjectToWorldEXT * vec4(posModelSpace, 1.f));

	// Interpolate normal of hitpoint
	const vec3 normalModelSpace = v0.Normal * barycentricCoords.x + v1.Normal * barycentricCoords.y + v2.Normal * barycentricCoords.z;
	const vec3 tangentModelSpace = v0.Tangent * barycentricCoords.x + v1.Tangent * barycentricCoords.y + v2.Tangent * barycentricCoords.z;
	const mat3 modelMatTransposedInverse = transpose(mat3(mat4x3(gl_WorldToObjectEXT)));
	vec3 normalWorldSpace = normalize(modelMatTransposedInverse * normalModelSpace);
	const vec3 tangentWorldSpace = normalize(tangentModelSpace);

	mat3 TBN = CalculateTBN(normalWorldSpace, tangentWorldSpace);

	normalWorldSpace = ApplyNormalMap(TBN, probe);

	vec3 directLight = CollectLight(posWorldSpace, normalWorldSpace, material, probe);
	float rayDist = length(posWorldSpace - gl_WorldRayOriginEXT);
	ProbeMatInPayload.hit.Distance = length(posWorldSpace - gl_WorldRayOriginEXT);

	vec3 vtxWorld[3];
	vtxWorld[0] = (gl_ObjectToWorldEXT * vec4(v0.Pos, 1.f)).xyz;
	vtxWorld[1] = (gl_ObjectToWorldEXT * vec4(v1.Pos, 1.f)).xyz;
	vtxWorld[2] = (gl_ObjectToWorldEXT * vec4(v2.Pos, 1.f)).xyz;

	vec3 faceDir = cross(vtxWorld[0] - vtxWorld[1], vtxWorld[2] - vtxWorld[1]);
	if(dot(faceDir, gl_WorldRayDirectionEXT) > 0) {
		ProbeMatInPayload.hit.Radiance = directLight + probe.EmissiveColor;
	} else {
		ProbeMatInPayload.hit.Radiance = vec3(0);
	}
}
