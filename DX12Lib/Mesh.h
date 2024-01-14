#pragma once
#include "Renderer.h"

struct D3DBuffer
{
    ComPtr<ID3D12Resource> resource;
    D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptorHandle;
    D3D12_GPU_DESCRIPTOR_HANDLE gpuDescriptorHandle;
};

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
    typedef WORD IndexType;

    void startUpload(Renderer& renderer, VertexPosColor* vertices, UINT inVertexCount, IndexType* indices, UINT inIndexCount);

    void end();

    void freeData();

    UINT vertexCount = 0;
    UINT indexCount = 0;

    D3DBuffer vertexBuffer;
//    ComPtr<ID3D12Resource> vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView;

    D3DBuffer indexBuffer;
//    ComPtr<ID3D12Resource> indexBuffer;
    D3D12_INDEX_BUFFER_VIEW indexBufferView;

private:
    void init();

    ComPtr<ID3D12Resource> intermediateVertexBuffer;
    ComPtr<ID3D12Resource> intermediateIndexBuffer;
};

