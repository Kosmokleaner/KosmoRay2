#pragma once
#include "global.h"


// GPU texture or buffer
class DataBlock
{
public:

    ComPtr<ID3D12Resource> m_resource;
    D3D12_GPU_DESCRIPTOR_HANDLE m_UAVGpuDescriptor = {};

    void Reset();

    void Load(Renderer& renderer, const char* fileName);

	// set m_desc before
    void CreateUAV(Renderer& renderer);

	D3D12_RESOURCE_DESC m_desc = {};

private:
    UINT m_UAVDescriptorHeapIndex = UINT_MAX;
};

