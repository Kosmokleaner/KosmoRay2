#include "DescriptorHeap.h"
#include "Renderer.h"
#include <d3dx12.h>

UINT DescriptorHeap::AllocateDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE* cpuDescriptor, UINT descriptorIndexToUse)
{
    auto descriptorHeapCpuBase = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
    if (descriptorIndexToUse >= maxSize)
    {
        descriptorIndexToUse = currentSize++;
    }
    *cpuDescriptor = CD3DX12_CPU_DESCRIPTOR_HANDLE(descriptorHeapCpuBase, descriptorIndexToUse, currentSize);
    return descriptorIndexToUse;
}

void DescriptorHeap::Reset()
{
    descriptorHeap.Reset();
    currentSize = 0;
}

void DescriptorHeap::CreateDescriptorHeap(Renderer& renderer, UINT inMaxSize, D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags)
{
    maxSize = inMaxSize;

    auto device = renderer.device;

    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.Type = type;
    desc.NumDescriptors = inMaxSize;
    desc.Flags = flags;
    desc.NodeMask = 0;

    device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descriptorHeap));
    NAME_D3D12_OBJECT(descriptorHeap);

    maxSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}