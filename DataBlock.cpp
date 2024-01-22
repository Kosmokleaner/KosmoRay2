#include "DataBlock.h"
#include "Renderer.h"

void DataBlock::Reset()
{
    m_UAVDescriptorHeapIndex = UINT_MAX;
} 

void DataBlock::CreateUAV(Renderer& renderer)
{
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
