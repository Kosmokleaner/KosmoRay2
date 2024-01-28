#include "DataBlock.h"
#include "Renderer.h"

#define STB_IMAGE_IMPLEMENTATION
#include "external/stb_image.h"

//#pragma comment(lib, "external/DirectXTK/lib/x64/Release/DirectXTK12.lib")
#pragma comment(lib, "external/DirectXTK/lib/x64/Debug/DirectXTK12.lib")
#include "external/DirectXTK/include/ResourceUploadBatch.h"

void DataBlock::Reset()
{
    m_UAVDescriptorHeapIndex = UINT_MAX;
}


void DataBlock::Load(Renderer& renderer, const char* fileName)
{
//    auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto defaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

    auto commandQueue = renderer.directCommandQueue;
//    auto commandQueue = renderer.copyCommandQueue;
    ID3D12GraphicsCommandList2* commandList = commandQueue->GetCommandList().Get();


    D3D12_RESOURCE_DESC desc = {};
    desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.MipLevels = 1;
    desc.Width = 256;
    desc.Height = 256;
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

    
/*
    // now we create an upload heap to upload our texture to the GPU
    ThrowIfFailed(renderer.device->CreateCommittedResource(
        &uploadHeapProperties,
        D3D12_HEAP_FLAG_NONE,
        &sdesc, // resource description for a buffer (storing the image data in this heap just to copy to the default heap)
        D3D12_RESOURCE_STATE_GENERIC_READ, // We will copy the contents from this heap to the default heap above
        nullptr,
        IID_PPV_ARGS(&textureBufferUploadHeap)));
*/
    ThrowIfFailed(renderer.device->CreateCommittedResource(
        &defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_resource)));
    NAME_D3D12_OBJECT(m_resource);

    
//    std::vector<uint32> mem;
    uint32* mem = (uint32*)_aligned_malloc(desc.Width * desc.Height * 4, 256);

//    mem.resize(desc.Width * desc.Height, 0xff00ff00);
    for(int y = 0; y < desc.Height; ++y)
    for (int x = 0; x < desc.Width; ++x)
    {
        mem[x + y * desc.Width] = (255-x) | (y << 8);
    }

    ResourceUploadBatch resourceUpload(renderer.device.Get());

    
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

    _aligned_free(mem);
/*
    // Copy data to upload heap
    UINT8* pUploadData = 0;
    CD3DX12_RANGE readRange(0, 0);
    ThrowIfFailed(textureBufferUploadHeap->Map(0, &readRange, reinterpret_cast<void**>(&pUploadData)));


    memcpy(pUploadData, textureData.pData, textureData.SlicePitch);
    textureBufferUploadHeap->Unmap(0, nullptr);

    assert(textureData.RowPitch);

    // Now we copy the upload buffer contents to the default heap
    UpdateSubresources(renderer.copyCommandList, m_resource.Get(), textureBufferUploadHeap.Get(), 0, 0, 1, &textureData);

    // transition the texture default heap to a pixel shader resource (we will be sampling from this heap in the pixel shader to get the color of pixels)
    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_resource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    renderer.copyCommandList->ResourceBarrier(1, &barrier);
*/

/*

// Create texture
    D3D12_HEAP_PROPERTIES heapPropertiesDefault = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    D3D12_RESOURCE_DESC resourceDescDefault = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, 256, 256, 1, 1);
    HRESULT hr = renderer.device->CreateCommittedResource(
        &heapPropertiesDefault,
        D3D12_HEAP_FLAG_NONE,
        &resourceDescDefault,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&m_resource)
    );

    UINT64 imageSize = resourceDescDefault.Width * resourceDescDefault.Height * Renderer::GetFormatBitsPerPixel(resourceDescDefault.Format);

    // Create upload heap
    ComPtr<ID3D12Resource> uploadHeap;
    D3D12_HEAP_PROPERTIES heapPropertiesUpload = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    D3D12_RESOURCE_DESC resourceDescUpload = CD3DX12_RESOURCE_DESC::Buffer(imageSize);
    hr = renderer.device->CreateCommittedResource(
        &heapPropertiesUpload,
        D3D12_HEAP_FLAG_NONE,
        &resourceDescUpload,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&uploadHeap)
    );

    // Copy data to upload heap
    UINT8* pUploadData;
    CD3DX12_RANGE readRange(0, 0);
    hr = uploadHeap->Map(0, &readRange, reinterpret_cast<void**>(&pUploadData));

//todo    memcpy(pUploadData, imageData, imageSize);
    uploadHeap->Unmap(0, nullptr);

    std::vector<uint32> mem;
    mem.resize(resourceDescDefault.Width * resourceDescDefault.Height, 0xff00ff00);

    // Copy data from upload heap to default heap (texture)
    D3D12_SUBRESOURCE_DATA subresourceData = {};
    subresourceData.pData = mem.data();
    subresourceData.RowPitch = 256 * 4; // Assuming 4 bytes per pixel (R8G8B8A8_UNORM)
    subresourceData.SlicePitch = subresourceData.RowPitch * 256;

    UpdateSubresources(renderer.copyCommandList, m_resource.Get(), uploadHeap.Get(), 0, 0, 1, &subresourceData);

    // Transition the texture to a shader resource state
    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_resource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    renderer.copyCommandList->ResourceBarrier(1, &barrier);

*/









   

    D3D12_CPU_DESCRIPTOR_HANDLE uavDescriptorHandle;

    m_UAVDescriptorHeapIndex = renderer.descriptorHeap.AllocateDescriptor(&uavDescriptorHandle, m_UAVDescriptorHeapIndex);

    D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};

    UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
    renderer.device->CreateUnorderedAccessView(m_resource.Get(), nullptr, &UAVDesc, uavDescriptorHandle);

    m_UAVGpuDescriptor = CD3DX12_GPU_DESCRIPTOR_HANDLE(
        renderer.descriptorHeap.descriptorHeap->GetGPUDescriptorHandleForHeapStart(),
        m_UAVDescriptorHeapIndex,
        renderer.descriptorHeap.maxSize);

//    auto fenceValue = commandQueue->ExecuteCommandList(commandList);
//    commandQueue->WaitForFenceValue(fenceValue);
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
