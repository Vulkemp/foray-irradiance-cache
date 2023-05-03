#version 460
#extension GL_GOOGLE_include_directive : enable // Include files
#extension GL_EXT_ray_tracing : enable // Raytracing
#extension GL_EXT_nonuniform_qualifier : enable // Required for asserting that some array indexing is done with non-uniform indices

// Include structs and bindings

#include "rt_common/bindpoints.glsl" // Bindpoints (= descriptor set layout)
#include "common/materialbuffer.glsl" // Material buffer for material information and texture array
#include "rt_common/geometrymetabuffer.glsl" // GeometryMeta information
#include "rt_common/geobuffers.glsl" // Vertex and index buffer aswell as accessor methods
#include "common/normaltbn.glsl" // Normal calculation in tangent space
#include "common/lcrng.glsl"
#include "rt_common/tlas.glsl" // Binds Top Level Acceleration Structure

#define BIND_SIMPLIFIEDLIGHTARRAY 11
#include "rt_common/simplifiedlights.glsl"

#include "shading/constants.glsl"
#include "shading/sampling.glsl"
#include "shading/material.glsl"

// Declare hitpayloads

#define HITPAYLOAD_IN
#define HITPAYLOAD_OUT
#include "rt_common/payload.glsl"

#include "../visitest/visitest.glsl"
const VisiTestConfig visiTestConfig = {1, 0, 1};

#define BIND_IN_IRRADIANCE_CACHE 12
#include "../irradiancecache/irradiancecache.glsl"

hitAttributeEXT vec2 attribs; // Barycentric coordinates

// Offsets a ray origin slightly away from the surface to prevent self shadowing
void CorrectOrigin(inout vec3 origin, vec3 normal, float nDotL)
{
    float correctorLength = clamp((1.0 - nDotL) * 0.005, 0, 1);
    origin += normal * correctorLength;
}

vec3 CollectDirectLight(in vec3 pos, in vec3 normal, in MaterialBufferObject material, in MaterialProbe probe)
{
    // Do a maximum of 5 light tests (since each is a ray cast, which is quite expensive)
    const uint lightTestCount = min(5, SimplifiedLights.Count);

    vec3 directLightSum = vec3(0);
    int directLightWeight = 0;

    for (uint i = 0; i < lightTestCount; i++)
    {
        // Randomly select a light source
        lcgUint(ReturnPayload.Seed);
        SimplifiedLight light = SimplifiedLights.Array[ReturnPayload.Seed % SimplifiedLights.Count];

        vec3 origin = pos;
        vec3 dir = vec3(0);
        float len = 0;
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
        }
        float nDotL = dot(dir, normal);
        CorrectOrigin(origin, normal, nDotL);
        

        if (nDotL > 0) // If light source is not behind the surface ...
        {
            if (!performVisiTest(visiTestConfig, origin, 0.001, dir, len)) // If light source is visible ...
            {
                HitSample hit;
                hit.Normal = normal;
                hit.wOut = -gl_WorldRayDirectionEXT;
                hit.wIn = dir;
                hit.wHalf = normalize(hit.wOut + hit.wIn);

                vec3 reflection = (ReturnPayload.Attenuation * light.Intensity * light.Color * EvaluateMaterial(hit, material, probe)) / (4 * PI); // Calculate light reflected

                if (light.Type == SimplifiedLightType_Point)
                {
                    reflection /= (len * len);
                }

                directLightSum += reflection;
                directLightWeight += 1;
            }
        }
    }

    if (directLightWeight > 0)
    {
        return directLightSum / directLightWeight;
    }
    else
    {
        return vec3(0);
    }
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

    vec3 directLight = CollectDirectLight(posWorldSpace, normalWorldSpace, material, probe);
    float rayDist = length(posWorldSpace - gl_WorldRayOriginEXT);
    ReturnPayload.Radiance = directLight + probe.EmissiveColor;
    ReturnPayload.Distance = length(posWorldSpace - gl_WorldRayOriginEXT);

    ReturnPayload.Radiance *= readIrradianceCacheWorldSpace(posWorldSpace).xyz;
}
