#include "Mesh.h"
#include "Helpers.h"


void Mesh::startUpload(Renderer& renderer, VertexPosColor* vertices, UINT inVertexCount, IndexType* indices, UINT inIndexCount)
{
    vertexCount = inVertexCount;
    indexCount = inIndexCount;

#if MESH_UPLOAD_METHOD == 0
    renderer.UpdateBufferResource(renderer.copyCommandList, &vertexBuffer.resource, &intermediateVertexBuffer, inVertexCount, sizeof(VertexPosColor), vertices);
    renderer.UpdateBufferResource(renderer.copyCommandList, &indexBuffer.resource, &intermediateIndexBuffer, indexCount, sizeof(IndexType), indices);
#elif MESH_UPLOAD_METHOD == 1
    AllocateUploadBuffer(renderer.device.Get(), vertices, vertexCount * sizeof(VertexPosColor), &vertexBuffer.resource);
    AllocateUploadBuffer(renderer.device.Get(), indices, indexCount * sizeof(IndexType), &indexBuffer.resource);
#endif

    NAME_D3D12_OBJECT(vertexBuffer.resource);
    NAME_D3D12_OBJECT(indexBuffer.resource);

   // Vertex buffer is passed to the shader along with index buffer as a descriptor range.
//   UINT descriptorIndexIB = CreateBufferSRV(&mesh.indexBuffer, (UINT)indexBuffer.size(), 2);

    init();
}

void Mesh::end()
{
#if MESH_UPLOAD_METHOD == 0
    intermediateVertexBuffer.Detach();
    intermediateIndexBuffer.Detach();
#endif
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

void Mesh::Reset()
{
    indexBuffer.resource.Reset();
    vertexBuffer.resource.Reset();
    bottomLevelAccelerationStructure.Reset();
}