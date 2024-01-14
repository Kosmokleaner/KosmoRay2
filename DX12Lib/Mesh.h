#pragma once
#include "Renderer.h"

// 0:UpdateBufferResource / 1:AllocateUploadBuffer(less code complexity, no commandlist need)
#define MESH_UPLOAD_METHOD 1

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

    // like FreeData / ReleaseDeviceDependentResources
    void Reset();

    UINT vertexCount = 0;
    UINT indexCount = 0;

    D3DBuffer vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView;

    D3DBuffer indexBuffer;
    D3D12_INDEX_BUFFER_VIEW indexBufferView;

    ComPtr<ID3D12Resource> bottomLevelAccelerationStructure;

private:
    void init();

#if MESH_UPLOAD_METHOD == 0
    ComPtr<ID3D12Resource> intermediateVertexBuffer;
    ComPtr<ID3D12Resource> intermediateIndexBuffer;
#endif
};

