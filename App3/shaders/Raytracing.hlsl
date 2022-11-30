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

#ifndef RAYTRACING_HLSL
#define RAYTRACING_HLSL

#include "RaytracingHlslCompat.h"
#include "cellular.hlsl"

RaytracingAccelerationStructure Scene : register(t0, space0);
RWTexture2D<float4> RenderTarget : register(u0);
ConstantBuffer<SceneConstantBuffer> g_sceneCB : register(b0);
ConstantBuffer<RayGenConstantBuffer> g_rayGenCB : register(b1);

typedef BuiltInTriangleIntersectionAttributes MyAttributes;

struct RayPayload
{
    float4 color;
    int count;
    float minT;
};

struct Ray
{
    float3 origin;
    float3 direction;
};

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
    RayPayload payload = { float4(0.2f, 0.2f, 0.2f, 0), 0, rayDesc.TMax };
    // closesthit
//        TraceRay(Scene, RAY_FLAG_CULL_BACK_FACING_TRIANGLES, ~0, 0, 1, 0, ray, payload);
    // closesthit
//        TraceRay(Scene, RAY_FLAG_FORCE_NON_OPAQUE, ~0, 0, 1, 0, ray, payload);
    // anyhit
//        TraceRay(Scene, RAY_FLAG_FORCE_NON_OPAQUE | RAY_FLAG_SKIP_CLOSEST_HIT_SHADER, ~0, 0, 1, 0, ray, payload);
//        TraceRay(Scene, RAY_FLAG_FORCE_NON_OPAQUE | RAY_FLAG_SKIP_CLOSEST_HIT_SHADER, ~0, 0, 1, 0, ray, payload);

        
        TraceRay(Scene, g_sceneCB.raytraceFlags, ~0, 0, 1, 0, rayDesc, payload);

//        TraceRay(Scene, section, ~0, 0, 1, 0, ray, payload);

    // Write the raytraced color to the output texture.
/*
    if(DispatchRaysIndex().y < 100)
        RenderTarget[DispatchRaysIndex().xy] = frac(payload.minT / 100.0f);
    else if (DispatchRaysIndex().y < 200)
        RenderTarget[DispatchRaysIndex().xy] = payload.color;
    else if (DispatchRaysIndex().y < 300)
        RenderTarget[DispatchRaysIndex().xy] = float4(0.1f,0.2f,0.3f, 1.0f) * payload.count;
    else 
*/
    {
        float col = 0;

        if(payload.count) {
            // world space position
            float3 pos = rayDesc.Origin + rayDesc.Direction * payload.minT;

            float2 cel = cellular(pos * 10.0f);
            col = 1.0f - cel.x;
         }

        if((payload.count % 2) == 1) {
            RenderTarget[DispatchRaysIndex().xy] = float4(col, 0, 0, 1);
        }
        else {
                RenderTarget[DispatchRaysIndex().xy] = payload.color;
//            RenderTarget[DispatchRaysIndex().xy] = float4(0, col, 0, 1);
        }
    }

//        if(section == 50 && (DispatchRaysIndex().x % 8) == 4)
//            RenderTarget[DispatchRaysIndex().xy] = float4(1,1,0,1);


    // hack
//    RenderTarget[DispatchRaysIndex().xy] = float4(1,1,1,1);

// crash to prove the shader is executed
//    for (; DispatchRaysIndex().x < 100000000;);
}

[shader("closesthit")]
void MyClosestHitShader(inout RayPayload payload, in MyAttributes attr)
{
    float3 barycentrics = float3(1 - attr.barycentrics.x - attr.barycentrics.y, attr.barycentrics.x, attr.barycentrics.y);
//    payload.color = float4(barycentrics, 1);
//    payload.color.r = barycentrics.x; // red
//    ++payload.count;
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
//    const float clipDepth = 3.0f;

    // [tMin..tMax]
    float t = RayTCurrent();

    // not animated
//    const float radius = 0.9f;
    // animated
    const float radius = 0.6f + 0.4f * sin(g_sceneCB.sceneParam0.y * 3.14159265f * 2.0f);

    // (tEnter, tExit)
    float2 tSphere = raySphereIntersect(WorldRayOrigin(), WorldRayDirection(), float3(-0.5f, 0, 0), radius);

//    if(length(pos) > clipDepth)
//    if (t > clipDepth) 
//    if (length(pos + float3(0.5f, 0, 0)) < 0.9f)
    if (t > tSphere.x && t < tSphere.y && tSphere.x != -1)
    {
        if (t < payload.minT) {
            payload.minT = t;
            float3 barycentrics = float3(1 - attr.barycentrics.x - attr.barycentrics.y, attr.barycentrics.x, attr.barycentrics.y);
            payload.color = float4(barycentrics, 1);


//            if(length(pos + float3(0.5f, 0, 0)) > 0.9f)
//                payload.color = float4(1,0,0,1);
        }
    }

    // count the number of triangle hit events after the sphere start
    // odd/even tells us if we are inside the surface
    if (t > tSphere.x && tSphere.x != -1) {
        ++payload.count;

        if(HitKind() != HIT_KIND_TRIANGLE_FRONT_FACE)
            if (tSphere.x < payload.minT)
                payload.minT = tSphere.x;
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