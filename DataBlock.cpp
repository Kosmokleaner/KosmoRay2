#include "DataBlock.h"
#include "Renderer.h"

#define STB_IMAGE_IMPLEMENTATION
#include "external/stb_image.h"

#ifdef _DEBUG
    #pragma comment(lib, "external/DirectXTK/lib/x64/Debug/DirectXTK12.lib")
#else
    #pragma comment(lib, "external/DirectXTK/lib/x64/Release/DirectXTK12.lib")
#endif

#include "external/DirectXTK/include/ResourceUploadBatch.h"

void DataBlock::Reset()
{
    m_UAVDescriptorHeapIndex = UINT_MAX;
}


void DataBlock::Load(Renderer& renderer, const char* fileName)
{
    int width = 0, height = 0, n = 0;
    uint32 *mem = (uint32*)stbi_load(fileName, &width, &height, &n, 4);
    if(!mem)
    {
        assert(0);
        return;
    }

    auto commandQueue = renderer.directCommandQueue;
    ID3D12GraphicsCommandList2* commandList = commandQueue->GetCommandList().Get();

    D3D12_RESOURCE_DESC desc = {};
    desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.MipLevels = 1;
    desc.Width = width;
    desc.Height = height;
    desc.DepthOrArraySize = 1;
    desc.SampleDesc.Count = 1;
    desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    
    // https://www.braynzarsoft.net/viewtutorial/q16390-directx-12-textures-from-file

    UINT64 textureUploadBufferSize = 0;
    // this function gets the size an upload buffer needs to be to upload a texture to the gpu.
    // each row must be 256 byte aligned except for the last row, which can just be the size in bytes of the row
    // eg. textureUploadBufferSize = ((((width * numBytesPerPixel) + 255) & ~255) * (height - 1)) + (width * numBytesPerPixel);
    //textureUploadBufferSize = (((imageBytesPerRow + 255) & ~255) * (textureDesc.Height - 1)) + imageBytesPerRow;
    renderer.device->GetCopyableFootprints(&desc, 0, 1, 0, nullptr, nullptr, nullptr, &textureUploadBufferSize);

    ComPtr<ID3D12Resource> textureBufferUploadHeap;

    CD3DX12_RESOURCE_DESC sdesc = CD3DX12_RESOURCE_DESC::Buffer(textureUploadBufferSize);

    auto defaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    ThrowIfFailed(renderer.device->CreateCommittedResource(
        &defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_resource)));
    NAME_D3D12_OBJECT(m_resource);

    
    DirectX::ResourceUploadBatch resourceUpload(renderer.device.Get());
    
    resourceUpload.Begin();

    // store vertex buffer in upload heap
    D3D12_SUBRESOURCE_DATA textureData = {};
    textureData.pData = mem;
    textureData.RowPitch = (Renderer::GetFormatBitsPerPixel(desc.Format) * desc.Width) / 8;
    textureData.SlicePitch = textureData.RowPitch * desc.Height;

    resourceUpload.Upload(m_resource.Get(), 0, &textureData, 1);

    resourceUpload.Transition(
        m_resource.Get(),
        D3D12_RESOURCE_STATE_COPY_DEST,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

    auto uploadResourcesFinished = resourceUpload.End(commandQueue->GetD3D12CommandQueue().Get());

    uploadResourcesFinished.wait();






   

    D3D12_CPU_DESCRIPTOR_HANDLE uavDescriptorHandle;

    m_UAVDescriptorHeapIndex = renderer.descriptorHeap.AllocateDescriptor(&uavDescriptorHandle, m_UAVDescriptorHeapIndex);

    D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};

    UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
    renderer.device->CreateUnorderedAccessView(m_resource.Get(), nullptr, &UAVDesc, uavDescriptorHandle);

    m_UAVGpuDescriptor = CD3DX12_GPU_DESCRIPTOR_HANDLE(
        renderer.descriptorHeap.descriptorHeap->GetGPUDescriptorHandleForHeapStart(),
        m_UAVDescriptorHeapIndex,
        renderer.descriptorHeap.maxSize);



    stbi_image_free(mem);
}


void DataBlock::CreateUAV(Renderer& renderer, const D3D12_RESOURCE_DESC& uavDesc)
{
    auto defaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    ThrowIfFailed(renderer.device->CreateCommittedResource(
        &defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &uavDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr, IID_PPV_ARGS(&m_resource)));
    NAME_D3D12_OBJECT(m_resource);


    D3D12_CPU_DESCRIPTOR_HANDLE uavDescriptorHandle;

    m_UAVDescriptorHeapIndex = renderer.descriptorHeap.AllocateDescriptor(&uavDescriptorHandle, m_UAVDescriptorHeapIndex);

    D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};

    UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
    renderer.device->CreateUnorderedAccessView(m_resource.Get(), nullptr, &UAVDesc, uavDescriptorHandle);

    m_UAVGpuDescriptor = CD3DX12_GPU_DESCRIPTOR_HANDLE(
        renderer.descriptorHeap.descriptorHeap->GetGPUDescriptorHandleForHeapStart(),
        m_UAVDescriptorHeapIndex,
        renderer.descriptorHeap.maxSize);
}
