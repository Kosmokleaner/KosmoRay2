#include "Mesh.h"
#include "Helpers.h"


void Mesh::startUpload(Renderer& renderer, VertexPosColor* vertices, UINT inVertexCount, IndexType* indices, UINT inIndexCount)
{
    vertexCount = inVertexCount;
    indexCount = inIndexCount;

    // Upload vertex buffer data
    renderer.UpdateBufferResource(renderer.copyCommandList, &vertexBuffer.resource, &intermediateVertexBuffer, inVertexCount, sizeof(VertexPosColor), vertices);
    NAME_D3D12_OBJECT(vertexBuffer.resource);

    // Upload index buffer data
    renderer.UpdateBufferResource(renderer.copyCommandList, &indexBuffer.resource, &intermediateIndexBuffer, indexCount, sizeof(IndexType), indices);
    NAME_D3D12_OBJECT(indexBuffer.resource);

    init();
}

void Mesh::init()
{
    assert(vertexCount);
    assert(indexCount);

    // vertex buffer view
    vertexBufferView.BufferLocation = vertexBuffer.resource->GetGPUVirtualAddress();
    vertexBufferView.StrideInBytes = sizeof(VertexPosColor);
    vertexBufferView.SizeInBytes = vertexCount * vertexBufferView.StrideInBytes;

    // index buffer view
    indexBufferView.BufferLocation = indexBuffer.resource->GetGPUVirtualAddress();
    indexBufferView.Format = DXGI_FORMAT_R16_UINT;
    assert(sizeof(IndexType) == 2);
    indexBufferView.SizeInBytes = indexCount * sizeof(IndexType);
}

void Mesh::end() 
{
    intermediateVertexBuffer.Detach();
    intermediateIndexBuffer.Detach();
}

void Mesh::freeData()
{
    indexBuffer.resource.Reset();
    vertexBuffer.resource.Reset();
}