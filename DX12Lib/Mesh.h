#pragma once
#include "Game.h"
#include "Window.h"
#include "Camera.h"

#include "Mathlib.h"

#include <d3d12.h>
#undef min
#undef max

#include <d3dx12.h>

#include <wrl.h> // ComPtr<>
using namespace Microsoft::WRL;

// for now we only have one vertex format
struct VertexPosColor
{
    XMFLOAT3 Position;
    XMFLOAT3 Color;
};

// a triangle mesh with index and vertex buffer
class Mesh
{
public:

    ComPtr<ID3D12Resource> m_VertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;
    ComPtr<ID3D12Resource> m_IndexBuffer;
    D3D12_INDEX_BUFFER_VIEW m_IndexBufferView;

};

