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
    ComPtr<ID3D12Device2> device;
    //
    std::shared_ptr<CommandQueue> copyCommandQueue;
    // copyCommandQueue->GetCommandList()
    ID3D12GraphicsCommandList2* copyCommandList = {};
};