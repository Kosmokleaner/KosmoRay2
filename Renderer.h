#pragma once
#include "global.h"
#include "DX12Lib/Window.h"
#include "Camera.h"
#include "DX12Lib/CommandQueue.h"

struct D3DBuffer
{
    ComPtr<ID3D12Resource> resource;
    D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptorHandle = {};
    D3D12_GPU_DESCRIPTOR_HANDLE gpuDescriptorHandle = {};
};

class Renderer
{
public:

    // constructor
    Renderer() {}
    // destructor
    ~Renderer();

    // prevent copy constructor
    Renderer(Renderer const&) = delete;
    // prevent assignment
    Renderer& operator=(Renderer const&) = delete;

    //
    void init();

    // Create a GPU buffer
    void UpdateBufferResource(ID3D12GraphicsCommandList2* commandList,
        ID3D12Resource** pDestinationResource, ID3D12Resource** pIntermediateResource,
        size_t numElements, size_t elementSize, const void* bufferData,
        D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);

    //
    ComPtr<ID3D12Device2> device;
    ComPtr<ID3D12Device5> dxrDevice;

    std::shared_ptr<CommandQueue> directCommandQueue;
    std::shared_ptr<CommandQueue> computeCommandQueue;
    std::shared_ptr<CommandQueue> copyCommandQueue;


    ComPtr<IDXGIAdapter4> dxgiAdapter;

    bool tearingSupported = false;

    // --------------------------------------------

    //
    bool IsRayTracingSupported() const;
    //
    UINT GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE type) const;
    // @param elementSize 0 for R32 typless raw buffer
    // @return descriptor index
    UINT CreateBufferSRV(D3DBuffer* buffer, UINT numElements, UINT elementSize);
    // Flush all command queues.
    void Flush();

    // todo

    // copyCommandQueue->GetCommandList()
    ID3D12GraphicsCommandList2* copyCommandList = {};
    // used by CreateBufferSRV() and CreateRaytracingOutputResource()
    DescriptorHeap descriptorHeap;

    // get the number of bits per pixel for a dxgi format
    // @param 0 if not known
    static uint32 GetFormatBitsPerPixel(DXGI_FORMAT dxgiFormat);
    
private:
    //
    void Reset();
    //
    ComPtr<IDXGIAdapter4> GetAdapter(bool bUseWarp);
    //
    ComPtr<ID3D12Device2> CreateDevice(ComPtr<IDXGIAdapter4> adapter);
    //
    bool CheckTearingSupport();
};
