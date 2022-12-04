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

typedef float2 XMFLOAT2;
typedef float3 XMFLOAT3;
typedef float4 XMFLOAT4;
typedef float4 XMVECTOR;
typedef float4x4 XMMATRIX;
typedef uint UINT;

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
    XMMATRIX clipFromWorld;
    XMMATRIX worldFromClip;
    XMVECTOR cameraPosition;
    // .x:frac(time), y.:frac(time*0.1)
    XMVECTOR sceneParam0;
    uint raytraceFlags;
    uint dummy[3];
};

// see struct VertexPosColor
struct Vertex
{
    XMFLOAT3 position;
    XMFLOAT3 color;
};

#endif // RAYTRACINGHLSLCOMPAT_H