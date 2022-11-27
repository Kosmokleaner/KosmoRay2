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

    Ray ray = GenerateCameraRay(DispatchRaysIndex().xy, g_sceneCB.cameraPosition.xyz, g_sceneCB.worldFromClip);
    float3 origin = ray.origin;
    float3 rayDir = ray.direction;
//    float3 origin = float3(0, 0, -1);
//    float3 rayDir = float3(lerpValues.xy * 2.0f - 1.0f, 1);

//    if (IsInsideViewport(origin.xy, g_rayGenCB.stencil))
    {
        // Trace the ray.
        // Set the ray's extents.
        RayDesc ray;
        ray.Origin = origin;
        ray.Direction = rayDir;
        // Set TMin to a non-zero small value to avoid aliasing issues due to floating - point errors.
        // TMin should be kept small to prevent missing geometry at close contact areas.
        ray.TMin = 0.001;
        ray.TMax = 10000.0;
        RayPayload payload = { float4(0, 0, 0, 0) };
        TraceRay(Scene, RAY_FLAG_CULL_BACK_FACING_TRIANGLES, ~0, 0, 1, 0, ray, payload);

        // Write the raytraced color to the output texture.
        RenderTarget[DispatchRaysIndex().xy] = payload.color;
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
    payload.color = float4(barycentrics, 1);
}

[shader("miss")]
void MyMissShader(inout RayPayload payload)
{
    payload.color = float4(0, 0, 0, 1);
}

#endif // RAYTRACING_HLSL