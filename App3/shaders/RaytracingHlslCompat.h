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

#ifndef RAYTRACINGHLSLCOMPAT_H
#define RAYTRACINGHLSLCOMPAT_H

// 0:off / 1:on
#define ANTIALIASING 1

#ifdef __cplusplus
typedef float2 glm::vec2;
typedef float3 glm::vec3;
typedef float4 glm::vec4;
typedef float4 glm::vec4;
typedef float4x4 glm::mat4;
typedef uint UINT;
#endif

struct Viewport
{
    float left;
    float top;
    float right;
    float bottom;
};

struct RayGenConstantBuffer
{
    Viewport viewport;
    Viewport stencil;
};

struct SceneConstantBuffer
{
    float4x4 clipFromWorld;
    float4x4 worldFromClip;
    float4 cameraPosition;
    // .x:frac(time), y.:frac(time*0.1)
    float4 sceneParam0;
    uint raytraceFlags;
    uint FrameIndex;
    uint dummy[2];
};

// see struct VFormatFull
struct Vertex
{
    float3 position;
    float3 normal;
    float2 uv;
};

#endif // RAYTRACINGHLSLCOMPAT_H