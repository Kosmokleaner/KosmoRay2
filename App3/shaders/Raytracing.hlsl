//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

// 0:MyClosestHitShader barycentric triangle color
// 1:MyAnyHitShader CSG boolean mesh operation
// 2:AO
#define RAY_TRACING_EXPERIMENT 0

#ifndef RAYTRACING_HLSL
#define RAYTRACING_HLSL

#include "RaytracingHlslCompat.h"
#include "cellular.hlsl"
#include "Helper.hlsl"

#define NV_SHADER_EXTN_SLOT u1
#define NV_SHADER_EXTN_REGISTER_SPACE space1
#include "../../external/nv-api/nvHLSLExtns.h"

RaytracingAccelerationStructure Scene : register(t0, space0);
RWTexture2D<float4> RenderTarget : register(u0);
ConstantBuffer<SceneConstantBuffer> g_sceneCB : register(b0);
ConstantBuffer<RayGenConstantBuffer> g_rayGenCB : register(b1);

// sizeof(IndexType) 2:16bit, 4:32bit
#define INDEX_STRIDE 2

// sizeof(VFormatFull)
#define VERTEX_STRIDE (8*4)

// index buffer (element size is INDEX_STRIDE)
ByteAddressBuffer g_indices : register(t1);
StructuredBuffer<Vertex> g_vertices : register(t2);

// https://microsoft.github.io/DirectX-Specs/d3d/Raytracing.html
// { float2 barycentrics }
typedef BuiltInTriangleIntersectionAttributes MyAttributes;

// update App3.cpp if this gets larger
struct RayPayload
{
    float4 color;
    // in world space, normalized
    float3 normal;
    //
    uint primitiveIndex;
    // to count intersections after sphere start, to test if inside the object
    int count;
    // used like z buffer, the smallest valus is used
    float minT;
    float minTfront;
};

struct Ray
{
    float3 origin;
    float3 direction;
};

// @param primitiveIndex from PrimitiveIndex()
// @return triangle corner vertex indices
uint3 LoadIndexBuffer( uint primitiveIndex )
{
    const uint indexOffsetBytes = 0;    // for now
    // triangle corner vertex indices
    uint offsetBytes = indexOffsetBytes + primitiveIndex * (INDEX_STRIDE * 3);

    // needed?
    const uint dwordAlignedOffset = offsetBytes & ~3;

    const uint2 four16BitIndices = g_indices.Load2(dwordAlignedOffset);

    uint3 indices;

    if (dwordAlignedOffset == offsetBytes)
    {
        indices.x = four16BitIndices.x & 0xffff;
        indices.y = (four16BitIndices.x >> 16) & 0xffff;
        indices.z = four16BitIndices.y & 0xffff;
    }
    else
    {
        indices.x = (four16BitIndices.x >> 16) & 0xffff;
        indices.y = four16BitIndices.y & 0xffff;
        indices.z = (four16BitIndices.y >> 16) & 0xffff;
    }

    return indices;
}

inline Ray GenerateCameraRay(uint2 index, in float3 cameraPosition, in float4x4 worldFromClip)
{
    float2 xy = index + 0.5f; // center in the middle of the pixel.
    float2 screenPos = xy / DispatchRaysDimensions().xy * 2.0 - 1.0;

    // Invert Y for DirectX-style coordinates.
    screenPos.y = -screenPos.y;

    // a way to compute cameraPosition, need to rename the input parameter
//    float4 cameraPositionHom = mul(worldFromClip, float4(0, 0, 0, 1));
//    float3 cameraPosition = cameraPositionHom.xyz / cameraPositionHom.w;

    // Unproject the pixel coordinate into a world positon.
    float4 worldHom = mul(worldFromClip, float4(screenPos, 1, 1));
    float3 world = worldHom.xyz / worldHom.w;

    Ray ray;
    ray.origin = cameraPosition;
    ray.direction = normalize(world - ray.origin);

    return ray;
}

[shader("raygeneration")]
void MyRaygenShader()
{
    uint startTime = NvGetSpecial(NV_SPECIALOP_GLOBAL_TIMER_LO);

    float2 lerpValues = (float2)DispatchRaysIndex() / (float2)DispatchRaysDimensions();

    float fracTime = g_sceneCB.sceneParam0.x;

    uint section = DispatchRaysIndex().x / 8;

    Ray ray = GenerateCameraRay(DispatchRaysIndex().xy, g_sceneCB.cameraPosition.xyz, g_sceneCB.worldFromClip);
    float3 origin = ray.origin;
    float3 rayDir = ray.direction;
//    float3 origin = float3(0, 0, -1);
//    float3 rayDir = float3(lerpValues.xy * 2.0f - 1.0f, 1);

    // Trace the ray.
    // Set the ray's extents.
    RayDesc rayDesc;
    rayDesc.Origin = origin;
    rayDesc.Direction = rayDir;
    // Set TMin to a non-zero small value to avoid aliasing issues due to floating - point errors.
    // TMin should be kept small to prevent missing geometry at close contact areas.
    rayDesc.TMin = 0.001f;
    rayDesc.TMax = 10000.0f;
    RayPayload payload = { float4(0.2f, 0.2f, 0.2f, 0), float3(0, 0, 0), 0, 0, rayDesc.TMax, rayDesc.TMax };

#if RAY_TRACING_EXPERIMENT == 0
    // closesthit
    TraceRay(Scene, RAY_FLAG_CULL_BACK_FACING_TRIANGLES, ~0, 0, 1, 0, rayDesc, payload);
    RenderTarget[DispatchRaysIndex().xy] = float4(payload.normal * 0.5f + 0.5f, 1.0f); // face normal
//    RenderTarget[DispatchRaysIndex().xy] = float4(payload.color.rgb, 1.0f); // barycentrics
//    RenderTarget[DispatchRaysIndex().xy] = float4(IndexToColor(payload.primitiveIndex), 1); // unique color for each triangle


#elif RAY_TRACING_EXPERIMENT == 1
    TraceRay(Scene, g_sceneCB.raytraceFlags, ~0, 0, 1, 0, rayDesc, payload);
    RenderTarget[DispatchRaysIndex().xy] = float4(payload.normal * 0.5f + 0.5f, 1.0f); // normal

#elif RAY_TRACING_EXPERIMENT == 2
    float AO = 0.0f;
    TraceRay(Scene, RAY_FLAG_CULL_BACK_FACING_TRIANGLES, ~0, 0, 1, 0, rayDesc, payload);
    if (all(payload.normal != float3(0, 0, 0)))
    {
        uint rnd = initRand(DispatchRaysIndex(), 0x12345678);

        uint sampleCountAO = 20;
        AO = 1;
        for(int i = 0; i < sampleCountAO; ++i)
        {
//            float2  = nextRand2
//            float3 worldDir = getCosHemisphereSample(randSeed, worldNorm);

//            TraceRay(Scene, RAY_FLAG_CULL_BACK_FACING_TRIANGLES, ~0, 0, 1, 0, rayDesc, payload);
          //  if(all(payload.normal != float3(0, 0, 0)))
          //      AO += 1.0f / sampleCountAO;
        }
    }
    RenderTarget[DispatchRaysIndex().xy] = float4(AO, AO, AO, 1.0f);

#endif

    // closesthit
    //  TraceRay(Scene, RAY_FLAG_FORCE_NON_OPAQUE, ~0, 0, 1, 0, ray, payload);
    // anyhit
    //  TraceRay(Scene, RAY_FLAG_FORCE_NON_OPAQUE | RAY_FLAG_SKIP_CLOSEST_HIT_SHADER, ~0, 0, 1, 0, ray, payload);

        

    // Write the raytraced color to the output texture.

/*    if(DispatchRaysIndex().y < 100)
        RenderTarget[DispatchRaysIndex().xy] = frac(payload.minT / 100.0f);
    else if (DispatchRaysIndex().y < 200)
        RenderTarget[DispatchRaysIndex().xy] = payload.color;
    else if (DispatchRaysIndex().y < 300)
        RenderTarget[DispatchRaysIndex().xy] = float4(0.1f,0.2f,0.3f, 1.0f) * payload.count;
    else if (DispatchRaysIndex().y < 400)
 */
       uint endTime = NvGetSpecial(NV_SPECIALOP_GLOBAL_TIMER_LO);
//       float f = endTime * 0.1f;
//      RenderTarget[DispatchRaysIndex().xy] = float4(f,f,f, 1.0f);
//       RenderTarget[DispatchRaysIndex().xy] = float4(payload.normal * 0.5f + 0.5f, 1.0f);
 //   else
//        RenderTarget[DispatchRaysIndex().xy] = payload.color;

    if(0)
    {
        float col = 0;

        if(payload.count)
        {
            // world space position
            float3 pos = rayDesc.Origin + rayDesc.Direction * payload.minT;

            float2 cel = cellular(pos * 10.0f);
            col = 1.0f - cel.x;
        }



//        if((payload.count % 2) == 1) {
            RenderTarget[DispatchRaysIndex().xy] = float4(col, 0, 0, 1);
//        }
//        else 
//        {
//            RenderTarget[DispatchRaysIndex().xy] = payload.color;
//        }
    }
}

[shader("closesthit")]
void MyClosestHitShader(inout RayPayload payload, in MyAttributes attr)
{
    float3 barycentrics = float3(1 - attr.barycentrics.x - attr.barycentrics.y, attr.barycentrics.x, attr.barycentrics.y);
    payload.color = float4(barycentrics, 1); // color from barycentrics

    payload.primitiveIndex = PrimitiveIndex();

    // triangle corner vertex indices
    const uint3 ii = LoadIndexBuffer(PrimitiveIndex());

    float3 bary = float3(attr.barycentrics.x, attr.barycentrics.y, 1.0 - attr.barycentrics.x - attr.barycentrics.y);

    // position in object space
    const float3 p0 = g_vertices[ii.x].position;
    const float3 p1 = g_vertices[ii.y].position;
    const float3 p2 = g_vertices[ii.z].position;

    const float3 n0 = g_vertices[ii.x].normal;
    const float3 n1 = g_vertices[ii.y].normal;
    const float3 n2 = g_vertices[ii.z].normal;

    float3 osNormal = n0 + bary.x * (n1 - n0) + bary.y * (n2 - n0);

    // visualize position id as color, gourand shading
//                payload.color = float4((p0 + bary.x * (p1 - p0) + bary.y * (p2 - p0)), 1);

                // visualize indexbuffer id as color, gourand shading
//                const float4 vCol0 = float4(IndexToColor(ii.x), 1);
//                const float4 vCol1 = float4(IndexToColor(ii.y), 1);
//                const float4 vCol2 = float4(IndexToColor(ii.z), 1);
//                payload.color = vCol0 + bary.x * (vCol1 - vCol0) + bary.y * (vCol2 - vCol0);

    float3 triangleNormal = normalize(cross(p2 - p0, p1 - p0));

    float3 worldPosition = WorldRayOrigin() + WorldRayDirection() * RayTCurrent();
    
    float3 worldNormal = mul(osNormal, (float3x3)ObjectToWorld3x4());
    //float3 worldNormal = mul(triangleNormal, (float3x3)ObjectToWorld3x4());

    payload.normal = normalize(worldNormal);
}

// from https://gist.github.com/wwwtyro/beecc31d65d1004f5a9d
float2 raySphereIntersect(float3 r0, float3 rd, float3 s0, float sr) {
    // - r0: ray origin
    // - rd: normalized ray direction
    // - s0: sphere center
    // - sr: sphere radius
    // - Returns distance from r0 to first intersecion with sphere,
    //   or -1.0 if no intersection.
    float a = dot(rd, rd);
    float3 s0_r0 = r0 - s0;
    float b = 2.0 * dot(rd, s0_r0);
    float c = dot(s0_r0, s0_r0) - (sr * sr);
    if (b * b - 4.0 * a * c < 0.0) {
        return float2(-1, -1);
    }
    // todo: optimize
    float2 ret;
    ret.x = (-b - sqrt((b * b) - 4.0 * a * c)) / (2.0 * a);
    ret.y = (-b + sqrt((b * b) - 4.0 * a * c)) / (2.0 * a);
    return ret;
}

// Direct computational noise in GLSL Supplementary material
// from https://github.com/stegu/webgl-noise


[shader("anyhit")]
void MyAnyHitShader(inout RayPayload payload, in MyAttributes attr)
{
    // [tMin..tMax]
    float t = RayTCurrent();

    // not animated, 0.9f to clip suzanne
//    const float radius = 0.9f;
    // animated
    float radius = 0.6f + 0.4f * sin(g_sceneCB.sceneParam0.y * 3.14159265f * 2.0f);

    // (tEnter, tExit)
    const float3 sphereCenter = float3(-0.5f, 0, 0);
    float2 tSphere = raySphereIntersect(WorldRayOrigin(), WorldRayDirection(), sphereCenter, radius);

    if(tSphere.x != -1) 
    {
        // if ray triangle hit is in clip object
        if (t > tSphere.x && t < tSphere.y)
        {
            // z buffer the triangle intersection
            if (t < payload.minT)
            {
                payload.minT = t;
                payload.minTfront = t;
                float3 barycentrics = float3(1 - attr.barycentrics.x - attr.barycentrics.y, attr.barycentrics.x, attr.barycentrics.y);
                payload.color = float4(barycentrics, 1);

                // visualize triangleId as color, flat shading
//                payload.color = float4(IndexToColor(PrimitiveIndex()), 1);

                payload.normal = float3(0, 1, 0);

                const uint3 ii = LoadIndexBuffer(PrimitiveIndex());

                float3 bary = float3(attr.barycentrics.x, attr.barycentrics.y, 1.0 - attr.barycentrics.x - attr.barycentrics.y);

                // position in object space
                const float3 p0 = g_vertices[ii.x].position;
                const float3 p1 = g_vertices[ii.y].position;
                const float3 p2 = g_vertices[ii.z].position;
                // visualize position id as color, gourand shading
//                payload.color = float4((p0 + bary.x * (p1 - p0) + bary.y * (p2 - p0)), 1);

                // visualize indexbuffer id as color, gourand shading
//                const float4 vCol0 = float4(IndexToColor(ii.x), 1);
//                const float4 vCol1 = float4(IndexToColor(ii.y), 1);
//                const float4 vCol2 = float4(IndexToColor(ii.z), 1);
//                payload.color = vCol0 + bary.x * (vCol1 - vCol0) + bary.y * (vCol2 - vCol0);

                float3 triangleNormal = normalize(cross(p2 - p0, p1 - p0));

                float3 worldPosition = WorldRayOrigin() + WorldRayDirection() * RayTCurrent();
//                float3 worldNormal = mul(attr.normal, (float3x3)ObjectToWorld3x4());
                float3 worldNormal = mul(triangleNormal, (float3x3)ObjectToWorld3x4());

                payload.primitiveIndex = PrimitiveIndex();
                payload.normal = worldNormal;
    //            if(length(pos + float3(0.5f, 0, 0)) > 0.9f)
    //                payload.color = float4(1,0,0,1);
            }
        }

        if (t >= tSphere.x) {
            // count the number of triangle hit events after the sphere start
            // odd/even tells us if we are inside the surface
            ++payload.count;
        }

        if (t < tSphere.x) {
            // if hit is backfacing
//            if(HitKind() == HIT_KIND_TRIANGLE_FRONT_FACE)
                // z buffer the sphere entrance
                if (t < payload.minT) {
                    payload.minT = t;
                    payload.minTfront = tSphere.x;
                    float3 localPos = WorldRayOrigin() - sphereCenter + payload.minT * WorldRayDirection();
                    // world normal
                    payload.normal = normalize(-localPos);
                }
        }
    /*    if (t >= tSphere.x && tSphere.x != -1) {
            // count the number of triangle hit events after the sphere start
            // odd/even tells us if we are inside the surface
            ++payload.count;

            // if hit is backfacing
            if (HitKind() == HIT_KIND_TRIANGLE_FRONT_FACE)
                // z buffer the sphere entrance
                if (tSphere.x < payload.minT) {
                    payload.minT = tSphere.x;
                    float3 localPos = WorldRayOrigin() - sphereCenter + payload.minT * WorldRayDirection();
                    payload.normal = normalize(localPos);
                }
        }
    */
    }

    // do not stop ray intersection, also calls miss shader
    IgnoreHit();
}

[shader("miss")]
void MyMissShader(inout RayPayload payload)
{
//    payload.color = float4(0, 0, 0, 1);
}

#endif // RAYTRACING_HLSL