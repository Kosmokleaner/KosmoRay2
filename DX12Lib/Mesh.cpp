#include "Mesh.h"
#include "Helpers.h"

void Renderer::UpdateBufferResource(
    ID3D12GraphicsCommandList2* commandList,
    ID3D12Resource** pDestinationResource,
    ID3D12Resource** pIntermediateResource,
    size_t numElements, size_t elementSize, const void* bufferData,
    D3D12_RESOURCE_FLAGS flags)
{
    size_t bufferSize = numElements * elementSize;

    CD3DX12_HEAP_PROPERTIES a(D3D12_HEAP_TYPE_DEFAULT);
    CD3DX12_RESOURCE_DESC b = CD3DX12_RESOURCE_DESC::Buffer(bufferSize, flags);

    // Create a committed resource for the GPU resource in a default heap.
    ThrowIfFailed(device->CreateCommittedResource(
        &a,
        D3D12_HEAP_FLAG_NONE,
        &b,
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(pDestinationResource)));

    // Create an committed resource for the upload.
    if (bufferData)
    {
        CD3DX12_HEAP_PROPERTIES a(D3D12_HEAP_TYPE_UPLOAD);
        CD3DX12_RESOURCE_DESC b = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

        ThrowIfFailed(device->CreateCommittedResource(
            &a,
            D3D12_HEAP_FLAG_NONE,
            &b,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(pIntermediateResource)));

        {
            CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(*pDestinationResource, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);

            commandList->ResourceBarrier(1, &barrier);
        }

        D3D12_SUBRESOURCE_DATA subresourceData = {};
        subresourceData.pData = bufferData;
        subresourceData.RowPitch = bufferSize;
        subresourceData.SlicePitch = subresourceData.RowPitch;

        UpdateSubresources(commandList,
            *pDestinationResource, *pIntermediateResource,
            0, 0, 1, &subresourceData);
    }
}


void Mesh::startUpload(Renderer& renderer, VertexPosColor* vertices, UINT inVertexCount, IndexType* indices, UINT inIndexCount)
{
    vertexCount = inVertexCount;
    indexCount = inIndexCount;

    // Upload vertex buffer data
    renderer.UpdateBufferResource(renderer.copyCommandList, &vertexBuffer, &intermediateVertexBuffer, inVertexCount, sizeof(VertexPosColor), vertices);
    NAME_D3D12_OBJECT(vertexBuffer);

    // Upload index buffer data
    renderer.UpdateBufferResource(renderer.copyCommandList, &indexBuffer, &intermediateIndexBuffer, indexCount, sizeof(IndexType), indices);
    NAME_D3D12_OBJECT(indexBuffer);

    init();
}

void Mesh::init()
{
    assert(vertexCount);
    assert(indexCount);

    // vertex buffer view
    vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
    vertexBufferView.StrideInBytes = sizeof(VertexPosColor);
    vertexBufferView.SizeInBytes = vertexCount * vertexBufferView.StrideInBytes;

    // index buffer view
    indexBufferView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
    indexBufferView.Format = DXGI_FORMAT_R16_UINT;
    assert(sizeof(IndexType) == 2);
    indexBufferView.SizeInBytes = indexCount * sizeof(IndexType);
}

void Mesh::end() 
{
    intermediateVertexBuffer.Detach();
    intermediateIndexBuffer.Detach();
}