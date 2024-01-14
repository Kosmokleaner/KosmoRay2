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

// todo: move
#include "CommandQueue.h"
class Renderer
{
public:

    // Create a GPU buffer.
    void UpdateBufferResource(ID3D12GraphicsCommandList2* commandList,
        ID3D12Resource** pDestinationResource, ID3D12Resource** pIntermediateResource,
        size_t numElements, size_t elementSize, const void* bufferData,
        D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);


    //
    ID3D12Device2* device;
    //
    CommandQueue* m_CopyCommandQueue;
    // m_CopyCommandQueue->GetCommandList()
    ID3D12GraphicsCommandList2* copyCommandList = {};
};

// a triangle mesh with index and vertex buffer
class Mesh
{
public:
    typedef WORD IndexType;

    void startUpload(Renderer& renderer, VertexPosColor* vertices, UINT inVertexCount, IndexType* indices, UINT inIndexCount);

    void end();

    UINT vertexCount = 0;
    UINT indexCount = 0;

    ComPtr<ID3D12Resource> vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
    ComPtr<ID3D12Resource> indexBuffer;
    D3D12_INDEX_BUFFER_VIEW indexBufferView;

private:
    void init();

    ComPtr<ID3D12Resource> intermediateVertexBuffer;
    ComPtr<ID3D12Resource> intermediateIndexBuffer;
};

