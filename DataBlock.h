#pragma once

#include <d3d12.h>
#undef min
#undef max

#include <wrl.h> // ComPtr<>
using namespace Microsoft::WRL;

class Renderer;

// GPU texture or buffer
class DataBlock
{
public:

    ComPtr<ID3D12Resource> m_resource;
    D3D12_GPU_DESCRIPTOR_HANDLE m_UAVGpuDescriptor = {};

    void Reset();

    void CreateUAV(Renderer& renderer);

private:

    UINT m_UAVDescriptorHeapIndex = UINT_MAX;
};

