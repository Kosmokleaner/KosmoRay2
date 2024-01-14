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

#include <dxgi1_6.h> // IDXGIAdapter4

#include "CommandQueue.h"

class Renderer
{
public:

    void init();

    // Create a GPU buffer.
    void UpdateBufferResource(ID3D12GraphicsCommandList2* commandList,
        ID3D12Resource** pDestinationResource, ID3D12Resource** pIntermediateResource,
        size_t numElements, size_t elementSize, const void* bufferData,
        D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);


    //
    ComPtr<ID3D12Device2> device;

    std::shared_ptr<CommandQueue> directCommandQueue;
    std::shared_ptr<CommandQueue> computeCommandQueue;
    std::shared_ptr<CommandQueue> copyCommandQueue;


    ComPtr<IDXGIAdapter4> dxgiAdapter;

    bool tearingSupported = false;

    bool IsRayTracingSupported() const;

    // todo

    // copyCommandQueue->GetCommandList()
    ID3D12GraphicsCommandList2* copyCommandList = {};

private:
    ComPtr<IDXGIAdapter4> GetAdapter(bool bUseWarp);
    ComPtr<ID3D12Device2> CreateDevice(ComPtr<IDXGIAdapter4> adapter);
    bool CheckTearingSupport();
};
