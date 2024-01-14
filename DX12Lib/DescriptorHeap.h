#pragma once

#include <d3d12.h>
#undef min
#undef max

#include <dxgi1_5.h>

#include <wrl.h> // ComPtr<>
using namespace Microsoft::WRL;

#include "Helpers.h"

class Renderer;

class DescriptorHeap
{
public:
    ComPtr<ID3D12DescriptorHeap> descriptorHeap;
    // aka m_descriptorsAllocated
    UINT currentSize = 0;
    // m_descriptorHeap->GetDesc().NumDescriptors, aka m_descriptorSize
    UINT maxSize = 0;

    void Reset();

    // @param type e.g. D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV / D3D12_DESCRIPTOR_HEAP_TYPE_RTV
    void CreateDescriptorHeap(Renderer& renderer, UINT inSize, D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE);

    // Allocate a single descriptor and return its index. 
    // If the passed descriptorIndexToUse is valid, it will be used instead of allocating a new one.
    UINT AllocateDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE* cpuDescriptor, UINT descriptorIndexToUse = UINT_MAX);
};

