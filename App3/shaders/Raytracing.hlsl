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

bool IsInsideViewport(float2 p, Viewport viewport)
{
    return (p.x >= viewport.left && p.x <= viewport.right)
        && (p.y >= viewport.top && p.y <= viewport.bottom);
}

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
/*
    RAY_FLAG_NONE = 0x00,
//    RAY_FLAG_FORCE_OPAQUE = 0x01, // no any hit shader
    RAY_FLAG_FORCE_NON_OPAQUE = 0x02,
    RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH = 0x04,
    RAY_FLAG_SKIP_CLOSEST_HIT_SHADER = 0x08,
    RAY_FLAG_CULL_BACK_FACING_TRIANGLES = 0x10,
    RAY_FLAG_CULL_FRONT_FACING_TRIANGLES = 0x20,
    RAY_FLAG_CULL_OPAQUE = 0x40,
    RAY_FLAG_CULL_NON_OPAQUE = 0x80,
    RAY_FLAG_SKIP_TRIANGLES = 0x100,
    RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES = 0x200,
*/
//    if (IsInsideViewport(origin.xy, g_rayGenCB.stencil))
    {
        // Trace the ray.
        // Set the ray's extents.
        RayDesc ray;
        ray.Origin = origin;
        ray.Direction = rayDir;
        // Set TMin to a non-zero small value to avoid aliasing issues due to floating - point errors.
        // TMin should be kept small to prevent missing geometry at close contact areas.
        ray.TMin = 0.001f;
        ray.TMax = 10000.0f;
        RayPayload payload = { float4(0.2f, 0.2f, 0.2f, 0), 0, ray.TMax };
        // closesthit
//        TraceRay(Scene, RAY_FLAG_CULL_BACK_FACING_TRIANGLES, ~0, 0, 1, 0, ray, payload);
        // closesthit
//        TraceRay(Scene, RAY_FLAG_FORCE_NON_OPAQUE, ~0, 0, 1, 0, ray, payload);
        // anyhit
//        TraceRay(Scene, RAY_FLAG_FORCE_NON_OPAQUE | RAY_FLAG_SKIP_CLOSEST_HIT_SHADER, ~0, 0, 1, 0, ray, payload);
        TraceRay(Scene, RAY_FLAG_FORCE_NON_OPAQUE | RAY_FLAG_SKIP_CLOSEST_HIT_SHADER, ~0, 0, 1, 0, ray, payload);

//        TraceRay(Scene, section, ~0, 0, 1, 0, ray, payload);

        // Write the raytraced color to the output texture.
/*        if(DispatchRaysIndex().y < 100)
            RenderTarget[DispatchRaysIndex().xy] = frac(payload.minT / 100.0f);
        else if (DispatchRaysIndex().y < 200)
            RenderTarget[DispatchRaysIndex().xy] = payload.color;
        else if (DispatchRaysIndex().y < 300)
            RenderTarget[DispatchRaysIndex().xy] = float4(0.1f,0.2f,0.3f, 1.0f) * payload.count;
        else 
*/      {
            if((payload.count % 2) == 1)
                RenderTarget[DispatchRaysIndex().xy] = float4(1,1,0,1);
            else
                RenderTarget[DispatchRaysIndex().xy] = payload.color;
        }

//        if(section == 50 && (DispatchRaysIndex().x % 8) == 4)
//            RenderTarget[DispatchRaysIndex().xy] = float4(1,1,0,1);
    }
//    else
//    {
        // Render yellow outside the stencil window
//        RenderTarget[DispatchRaysIndex().xy] = float4(1, 1, 0, 1);
//    }


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


[shader("anyhit")]
void MyAnyHitShader(inout RayPayload payload, in MyAttributes attr)
{
    float clipDepth = 3.0f;

    float t = RayTCurrent();
    float3 pos = WorldRayOrigin() + WorldRayDirection() * t;

//    if(length(pos) > clipDepth)
    if (t > clipDepth) 
//    if (length(pos + float3(0.5f, 0, 0)) > 0.9f)
    {
        if (t < payload.minT) {
            payload.minT = t;
            float3 barycentrics = float3(1 - attr.barycentrics.x - attr.barycentrics.y, attr.barycentrics.x, attr.barycentrics.y);
            payload.color = float4(barycentrics, 1);

//            if(length(pos + float3(0.5f, 0, 0)) > 0.9f)
//                payload.color = float4(1,0,0,1);
        }
        ++payload.count;
    }
//    ++payload.count;

    // do not stop ray intersection, also calls miss shader
    IgnoreHit();
}

[shader("miss")]
void MyMissShader(inout RayPayload payload)
{
//    payload.color = float4(0, 0, 0, 1);
}

#endif // RAYTRACING_HLSL