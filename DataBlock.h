#pragma once
#include "global.h"


// GPU texture or buffer
class DataBlock
{
public:

    ComPtr<ID3D12Resource> m_resource;
    D3D12_GPU_DESCRIPTOR_HANDLE m_UAVGpuDescriptor = {};

    void Reset();

    void CreateUAV(Renderer& renderer, const D3D12_RESOURCE_DESC& uavDesc);

private:

    UINT m_UAVDescriptorHeapIndex = UINT_MAX;
};

