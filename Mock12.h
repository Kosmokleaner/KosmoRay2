#pragma once
#include <d3d12.h>

#include <wrl.h>
using namespace Microsoft::WRL;

// not yet wrapped:
// Mock12Device2 : ID3D12InfoQueue
// Mock12Device2 : ID3D12Device5
// 
// ID3D12CommandAllocator
// ID3D12PipelineState
// ID3D12Resource
// ID3D12Heap
// ID3D12DeviceChild
// ID3D12Pageable
// ID3D12RootSignature
// ID3D12Fence

struct Mock12Resource : public ID3D12Resource
{
    ULONG m_dwRef = 0;
    ComPtr<ID3D12Resource> redirect;

    Mock12Resource(ComPtr<ID3D12Resource>& inRedirect)
    {
        redirect = inRedirect;
    }

    BEGIN_INTERFACE
        virtual HRESULT STDMETHODCALLTYPE QueryInterface(
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject)
    {
        return redirect->QueryInterface(riid, ppvObject);
    }

    virtual ULONG STDMETHODCALLTYPE AddRef(void)
    {
        return ++m_dwRef;
    }

    virtual ULONG STDMETHODCALLTYPE Release(void)
    {
        if (--m_dwRef == 0)
        {
            delete this;
            return 0;
        }
        return m_dwRef;
    }

    //

    virtual HRESULT STDMETHODCALLTYPE GetPrivateData(
        _In_  REFGUID guid,
        _Inout_  UINT* pDataSize,
        _Out_writes_bytes_opt_(*pDataSize)  void* pData)
    {
        return redirect->GetPrivateData(guid, pDataSize, pData);
    }

    virtual HRESULT STDMETHODCALLTYPE SetPrivateData(
        _In_  REFGUID guid,
        _In_  UINT DataSize,
        _In_reads_bytes_opt_(DataSize)  const void* pData)
    {
        return redirect->SetPrivateData(guid, DataSize, pData);
    }

    virtual HRESULT STDMETHODCALLTYPE SetPrivateDataInterface(
        _In_  REFGUID guid,
        _In_opt_  const IUnknown* pData)
    {
        return redirect->SetPrivateDataInterface(guid, pData);
    }

    virtual HRESULT STDMETHODCALLTYPE SetName(
        _In_z_  LPCWSTR Name)
    {
        return redirect->SetName(Name);
    }

    //

    virtual HRESULT STDMETHODCALLTYPE GetDevice(
        REFIID riid,
        _COM_Outptr_opt_  void** ppvDevice)
    {
        return redirect->GetDevice(riid, ppvDevice);
    }

    //

    virtual HRESULT STDMETHODCALLTYPE Map(
        UINT Subresource,
        _In_opt_  const D3D12_RANGE* pReadRange,
        _Outptr_opt_result_bytebuffer_(_Inexpressible_("Dependent on resource"))  void** ppData)
    {
        return redirect->Map(Subresource, pReadRange, ppData);
    }

    virtual void STDMETHODCALLTYPE Unmap(
        UINT Subresource,
        _In_opt_  const D3D12_RANGE* pWrittenRange)
    {
        return redirect->Unmap(Subresource, pWrittenRange);
    }

#if defined(_MSC_VER) || !defined(_WIN32)
    virtual D3D12_RESOURCE_DESC STDMETHODCALLTYPE GetDesc(void)
    {
        return redirect->GetDesc();
    }

#else
    virtual D3D12_RESOURCE_DESC* STDMETHODCALLTYPE GetDesc(
        D3D12_RESOURCE_DESC* RetVal)
    {
        return redirect->GetDesc(RetVal);
    }
#endif

    virtual D3D12_GPU_VIRTUAL_ADDRESS STDMETHODCALLTYPE GetGPUVirtualAddress(void)
    {
        return redirect->GetGPUVirtualAddress();
    }

    virtual HRESULT STDMETHODCALLTYPE WriteToSubresource(
        UINT DstSubresource,
        _In_opt_  const D3D12_BOX* pDstBox,
        _In_  const void* pSrcData,
        UINT SrcRowPitch,
        UINT SrcDepthPitch)
    {
        return redirect->WriteToSubresource(DstSubresource, pDstBox, pSrcData, SrcRowPitch, SrcDepthPitch);
    }

    virtual HRESULT STDMETHODCALLTYPE ReadFromSubresource(
        _Out_  void* pDstData,
        UINT DstRowPitch,
        UINT DstDepthPitch,
        UINT SrcSubresource,
        _In_opt_  const D3D12_BOX* pSrcBox)
    {
        return redirect->ReadFromSubresource(pDstData, DstRowPitch, DstDepthPitch, SrcSubresource, pSrcBox);
    }

    virtual HRESULT STDMETHODCALLTYPE GetHeapProperties(
        _Out_opt_  D3D12_HEAP_PROPERTIES* pHeapProperties,
        _Out_opt_  D3D12_HEAP_FLAGS* pHeapFlags)
    {
        return redirect->GetHeapProperties(pHeapProperties, pHeapFlags);
    }
};

struct Mock12Device2 : public ID3D12Device2
{
    ULONG m_dwRef = 0;
    ComPtr<ID3D12Device2> redirect;

    Mock12Device2(ComPtr<ID3D12Device2>& inRedirect)
    {
        redirect = inRedirect;
    }

    BEGIN_INTERFACE
        virtual HRESULT STDMETHODCALLTYPE QueryInterface(
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject)
    {
        if(riid == __uuidof(ID3D12InfoQueue))
            int d = 0;
        else if (riid == __uuidof(ID3D12Device5))
            int d = 0;
        else
            int d = 0;

        return redirect->QueryInterface(riid, ppvObject);
    }

    virtual ULONG STDMETHODCALLTYPE AddRef(void)
    {
        return ++m_dwRef;
    }

    virtual ULONG STDMETHODCALLTYPE Release(void)
    {
        if (--m_dwRef == 0)
        {
            delete this;
            return 0;
        }
        return m_dwRef;
    }

    // 

    virtual HRESULT STDMETHODCALLTYPE GetPrivateData(
        _In_  REFGUID guid,
        _Inout_  UINT* pDataSize,
        _Out_writes_bytes_opt_(*pDataSize)  void* pData)
    {
        return redirect->GetPrivateData(guid, pDataSize, pData);
    }

    virtual HRESULT STDMETHODCALLTYPE SetPrivateData(
        _In_  REFGUID guid,
        _In_  UINT DataSize,
        _In_reads_bytes_opt_(DataSize)  const void* pData)
    {
        return redirect->SetPrivateData(guid, DataSize, pData);
    }


    virtual HRESULT STDMETHODCALLTYPE SetPrivateDataInterface(
        _In_  REFGUID guid,
        _In_opt_  const IUnknown* pData)
    {
        return redirect->SetPrivateDataInterface(guid, pData);
    }

    virtual HRESULT STDMETHODCALLTYPE SetName(
        _In_z_  LPCWSTR Name)
    {
        return redirect->SetName(Name);
    }

    //

    virtual UINT STDMETHODCALLTYPE GetNodeCount(void)
    {
        return redirect->GetNodeCount();
    }

    virtual HRESULT STDMETHODCALLTYPE CreateCommandQueue(
        _In_  const D3D12_COMMAND_QUEUE_DESC* pDesc,
        REFIID riid,
        _COM_Outptr_  void** ppCommandQueue)
    {
        return redirect->CreateCommandQueue(pDesc, riid, ppCommandQueue);
    }

    virtual HRESULT STDMETHODCALLTYPE CreateCommandAllocator(
        _In_  D3D12_COMMAND_LIST_TYPE type,
        REFIID riid,
        _COM_Outptr_  void** ppCommandAllocator)
    {
        return redirect->CreateCommandAllocator(type, riid, ppCommandAllocator);
    }

    virtual HRESULT STDMETHODCALLTYPE CreateGraphicsPipelineState(
        _In_  const D3D12_GRAPHICS_PIPELINE_STATE_DESC* pDesc,
        REFIID riid,
        _COM_Outptr_  void** ppPipelineState)
    {
        return redirect->CreateGraphicsPipelineState(pDesc, riid, ppPipelineState);
    }

    virtual HRESULT STDMETHODCALLTYPE CreateComputePipelineState(
        _In_  const D3D12_COMPUTE_PIPELINE_STATE_DESC* pDesc,
        REFIID riid,
        _COM_Outptr_  void** ppPipelineState)
    {
        return redirect->CreateComputePipelineState(pDesc, riid, ppPipelineState);
    }

    virtual HRESULT STDMETHODCALLTYPE CreateCommandList(
        _In_  UINT nodeMask,
        _In_  D3D12_COMMAND_LIST_TYPE type,
        _In_  ID3D12CommandAllocator* pCommandAllocator,
        _In_opt_  ID3D12PipelineState* pInitialState,
        REFIID riid,
        _COM_Outptr_  void** ppCommandList)
    {
        return redirect->CreateCommandList(nodeMask, type, pCommandAllocator, pInitialState, riid, ppCommandList);
    }

    virtual HRESULT STDMETHODCALLTYPE CheckFeatureSupport(
        D3D12_FEATURE Feature,
        _Inout_updates_bytes_(FeatureSupportDataSize)  void* pFeatureSupportData,
        UINT FeatureSupportDataSize)
    {
        return redirect->CheckFeatureSupport(Feature, pFeatureSupportData, FeatureSupportDataSize);
    }

    virtual HRESULT STDMETHODCALLTYPE CreateDescriptorHeap(
        _In_  const D3D12_DESCRIPTOR_HEAP_DESC* pDescriptorHeapDesc,
        REFIID riid,
        _COM_Outptr_  void** ppvHeap)
    {
        return redirect->CreateDescriptorHeap(pDescriptorHeapDesc, riid, ppvHeap);
    }

    virtual UINT STDMETHODCALLTYPE GetDescriptorHandleIncrementSize(
        _In_  D3D12_DESCRIPTOR_HEAP_TYPE DescriptorHeapType)
    {
        return redirect->GetDescriptorHandleIncrementSize(DescriptorHeapType);
    }

    virtual HRESULT STDMETHODCALLTYPE CreateRootSignature(
        _In_  UINT nodeMask,
        _In_reads_(blobLengthInBytes)  const void* pBlobWithRootSignature,
        _In_  SIZE_T blobLengthInBytes,
        REFIID riid,
        _COM_Outptr_  void** ppvRootSignature)
    {
        return redirect->CreateRootSignature(nodeMask, pBlobWithRootSignature, blobLengthInBytes, riid, ppvRootSignature);
    }

    virtual void STDMETHODCALLTYPE CreateConstantBufferView(
        _In_opt_  const D3D12_CONSTANT_BUFFER_VIEW_DESC* pDesc,
        _In_  D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor)
    {
        redirect->CreateConstantBufferView(pDesc, DestDescriptor);
    }

    virtual void STDMETHODCALLTYPE CreateShaderResourceView(
        _In_opt_  ID3D12Resource* pResource,
        _In_opt_  const D3D12_SHADER_RESOURCE_VIEW_DESC* pDesc,
        _In_  D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor)
    {
        redirect->CreateShaderResourceView(pResource, pDesc, DestDescriptor);
    }

    virtual void STDMETHODCALLTYPE CreateUnorderedAccessView(
        _In_opt_  ID3D12Resource* pResource,
        _In_opt_  ID3D12Resource* pCounterResource,
        _In_opt_  const D3D12_UNORDERED_ACCESS_VIEW_DESC* pDesc,
        _In_  D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor)
    {
        redirect->CreateUnorderedAccessView(pResource, pCounterResource, pDesc, DestDescriptor);
    }

    virtual void STDMETHODCALLTYPE CreateRenderTargetView(
        _In_opt_  ID3D12Resource* pResource,
        _In_opt_  const D3D12_RENDER_TARGET_VIEW_DESC* pDesc,
        _In_  D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor)
    {
        redirect->CreateRenderTargetView(pResource, pDesc, DestDescriptor);
    }

    virtual void STDMETHODCALLTYPE CreateDepthStencilView(
        _In_opt_  ID3D12Resource* pResource,
        _In_opt_  const D3D12_DEPTH_STENCIL_VIEW_DESC* pDesc,
        _In_  D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor)
    {
        redirect->CreateDepthStencilView(pResource, pDesc, DestDescriptor);
    }

    virtual void STDMETHODCALLTYPE CreateSampler(
        _In_  const D3D12_SAMPLER_DESC* pDesc,
        _In_  D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor)
    {
        redirect->CreateSampler(pDesc, DestDescriptor);
    }

    virtual void STDMETHODCALLTYPE CopyDescriptors(
        _In_  UINT NumDestDescriptorRanges,
        _In_reads_(NumDestDescriptorRanges)  const D3D12_CPU_DESCRIPTOR_HANDLE* pDestDescriptorRangeStarts,
        _In_reads_opt_(NumDestDescriptorRanges)  const UINT* pDestDescriptorRangeSizes,
        _In_  UINT NumSrcDescriptorRanges,
        _In_reads_(NumSrcDescriptorRanges)  const D3D12_CPU_DESCRIPTOR_HANDLE* pSrcDescriptorRangeStarts,
        _In_reads_opt_(NumSrcDescriptorRanges)  const UINT* pSrcDescriptorRangeSizes,
        _In_  D3D12_DESCRIPTOR_HEAP_TYPE DescriptorHeapsType)
    {
        redirect->CopyDescriptors(NumDestDescriptorRanges, pDestDescriptorRangeStarts, pDestDescriptorRangeSizes, NumSrcDescriptorRanges, pSrcDescriptorRangeStarts, pSrcDescriptorRangeSizes, DescriptorHeapsType);
    }

    virtual void STDMETHODCALLTYPE CopyDescriptorsSimple(
        _In_  UINT NumDescriptors,
        _In_  D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptorRangeStart,
        _In_  D3D12_CPU_DESCRIPTOR_HANDLE SrcDescriptorRangeStart,
        _In_  D3D12_DESCRIPTOR_HEAP_TYPE DescriptorHeapsType)
    {
        redirect->CopyDescriptorsSimple(NumDescriptors, DestDescriptorRangeStart, SrcDescriptorRangeStart, DescriptorHeapsType);
    }

#if defined(_MSC_VER) || !defined(_WIN32)
    virtual D3D12_RESOURCE_ALLOCATION_INFO STDMETHODCALLTYPE GetResourceAllocationInfo(
        _In_  UINT visibleMask,
        _In_  UINT numResourceDescs,
        _In_reads_(numResourceDescs)  const D3D12_RESOURCE_DESC* pResourceDescs)
    {
        return redirect->GetResourceAllocationInfo(visibleMask, numResourceDescs, pResourceDescs);
    }
#else
    virtual D3D12_RESOURCE_ALLOCATION_INFO* STDMETHODCALLTYPE GetResourceAllocationInfo(
        D3D12_RESOURCE_ALLOCATION_INFO* RetVal,
        _In_  UINT visibleMask,
        _In_  UINT numResourceDescs,
        _In_reads_(numResourceDescs)  const D3D12_RESOURCE_DESC* pResourceDescs)
    {
        return redirect->GetResourceAllocationInfo(RetVal, visibleMask, numResourceDescs, pResourceDescs);
    }
#endif

#if defined(_MSC_VER) || !defined(_WIN32)
    virtual D3D12_HEAP_PROPERTIES STDMETHODCALLTYPE GetCustomHeapProperties(
        _In_  UINT nodeMask,
        D3D12_HEAP_TYPE heapType)
    {
        return redirect->GetCustomHeapProperties(nodeMask, heapType);
    }
#else
    virtual D3D12_HEAP_PROPERTIES* STDMETHODCALLTYPE GetCustomHeapProperties(
        D3D12_HEAP_PROPERTIES* RetVal,
        _In_  UINT nodeMask,
        D3D12_HEAP_TYPE heapType)
    {
        return redirect->GetCustomHeapProperties(RetVal, nodeMask, heapType);
    }
#endif

    virtual HRESULT STDMETHODCALLTYPE CreateCommittedResource(
        _In_  const D3D12_HEAP_PROPERTIES* pHeapProperties,
        D3D12_HEAP_FLAGS HeapFlags,
        _In_  const D3D12_RESOURCE_DESC* pDesc,
        D3D12_RESOURCE_STATES InitialResourceState,
        _In_opt_  const D3D12_CLEAR_VALUE* pOptimizedClearValue,
        REFIID riidResource,
        _COM_Outptr_opt_  void** ppvResource)
    {
        return redirect->CreateCommittedResource(pHeapProperties, HeapFlags, pDesc, InitialResourceState, pOptimizedClearValue, riidResource, ppvResource);
    }

    virtual HRESULT STDMETHODCALLTYPE CreateHeap(
        _In_  const D3D12_HEAP_DESC* pDesc,
        REFIID riid,
        _COM_Outptr_opt_  void** ppvHeap)
    {
        return redirect->CreateHeap(pDesc, riid, ppvHeap);
    }

    virtual HRESULT STDMETHODCALLTYPE CreatePlacedResource(
        _In_  ID3D12Heap* pHeap,
        UINT64 HeapOffset,
        _In_  const D3D12_RESOURCE_DESC* pDesc,
        D3D12_RESOURCE_STATES InitialState,
        _In_opt_  const D3D12_CLEAR_VALUE* pOptimizedClearValue,
        REFIID riid,
        _COM_Outptr_opt_  void** ppvResource)
    {
        return redirect->CreatePlacedResource(pHeap, HeapOffset, pDesc, InitialState, pOptimizedClearValue, riid, ppvResource);
    }

    virtual HRESULT STDMETHODCALLTYPE CreateReservedResource(
        _In_  const D3D12_RESOURCE_DESC* pDesc,
        D3D12_RESOURCE_STATES InitialState,
        _In_opt_  const D3D12_CLEAR_VALUE* pOptimizedClearValue,
        REFIID riid,
        _COM_Outptr_opt_  void** ppvResource)
    {
        return redirect->CreateReservedResource(pDesc, InitialState, pOptimizedClearValue, riid, ppvResource);
    }

    virtual HRESULT STDMETHODCALLTYPE CreateSharedHandle(
        _In_  ID3D12DeviceChild* pObject,
        _In_opt_  const SECURITY_ATTRIBUTES* pAttributes,
        DWORD Access,
        _In_opt_  LPCWSTR Name,
        _Out_  HANDLE* pHandle)
    {
        return redirect->CreateSharedHandle(pObject, pAttributes, Access, Name, pHandle);
    }

    virtual HRESULT STDMETHODCALLTYPE OpenSharedHandle(
        _In_  HANDLE NTHandle,
        REFIID riid,
        _COM_Outptr_opt_  void** ppvObj)
    {
        return redirect->OpenSharedHandle(NTHandle, riid, ppvObj);
    }

    virtual HRESULT STDMETHODCALLTYPE OpenSharedHandleByName(
        _In_  LPCWSTR Name,
        DWORD Access,
        /* [annotation][out] */
        _Out_  HANDLE* pNTHandle)
    {
        return redirect->OpenSharedHandleByName(Name, Access, pNTHandle);
    }

    virtual HRESULT STDMETHODCALLTYPE MakeResident(
        UINT NumObjects,
        _In_reads_(NumObjects)  ID3D12Pageable* const* ppObjects)
    {
        return redirect->MakeResident(NumObjects, ppObjects);
    }

    virtual HRESULT STDMETHODCALLTYPE Evict(
        UINT NumObjects,
        _In_reads_(NumObjects)  ID3D12Pageable* const* ppObjects)
    {
        return redirect->Evict(NumObjects, ppObjects);
    }

    virtual HRESULT STDMETHODCALLTYPE CreateFence(
        UINT64 InitialValue,
        D3D12_FENCE_FLAGS Flags,
        REFIID riid,
        _COM_Outptr_  void** ppFence)
    {
        return redirect->CreateFence(InitialValue, Flags, riid, ppFence);
    }

    virtual HRESULT STDMETHODCALLTYPE GetDeviceRemovedReason(void)
    {
        return redirect->GetDeviceRemovedReason();
    }

    virtual void STDMETHODCALLTYPE GetCopyableFootprints(
        _In_  const D3D12_RESOURCE_DESC* pResourceDesc,
        _In_range_(0, D3D12_REQ_SUBRESOURCES)  UINT FirstSubresource,
        _In_range_(0, D3D12_REQ_SUBRESOURCES - FirstSubresource)  UINT NumSubresources,
        UINT64 BaseOffset,
        _Out_writes_opt_(NumSubresources)  D3D12_PLACED_SUBRESOURCE_FOOTPRINT* pLayouts,
        _Out_writes_opt_(NumSubresources)  UINT* pNumRows,
        _Out_writes_opt_(NumSubresources)  UINT64* pRowSizeInBytes,
        _Out_opt_  UINT64* pTotalBytes)
    {
        redirect->GetCopyableFootprints(pResourceDesc, FirstSubresource, NumSubresources, BaseOffset, pLayouts, pNumRows, pRowSizeInBytes, pTotalBytes);
    }

    virtual HRESULT STDMETHODCALLTYPE CreateQueryHeap(
        _In_  const D3D12_QUERY_HEAP_DESC* pDesc,
        REFIID riid,
        _COM_Outptr_opt_  void** ppvHeap)
    {
        return redirect->CreateQueryHeap(pDesc, riid, ppvHeap);
    }

    virtual HRESULT STDMETHODCALLTYPE SetStablePowerState(
        BOOL Enable)
    {
        return redirect->SetStablePowerState(Enable);
    }

    virtual HRESULT STDMETHODCALLTYPE CreateCommandSignature(
        _In_  const D3D12_COMMAND_SIGNATURE_DESC* pDesc,
        _In_opt_  ID3D12RootSignature* pRootSignature,
        REFIID riid,
        _COM_Outptr_opt_  void** ppvCommandSignature)
    {
        return redirect->CreateCommandSignature(pDesc, pRootSignature, riid, ppvCommandSignature);
    }

    virtual void STDMETHODCALLTYPE GetResourceTiling(
        _In_  ID3D12Resource* pTiledResource,
        _Out_opt_  UINT* pNumTilesForEntireResource,
        _Out_opt_  D3D12_PACKED_MIP_INFO* pPackedMipDesc,
        _Out_opt_  D3D12_TILE_SHAPE* pStandardTileShapeForNonPackedMips,
        _Inout_opt_  UINT* pNumSubresourceTilings,
        _In_  UINT FirstSubresourceTilingToGet,
        _Out_writes_(*pNumSubresourceTilings)  D3D12_SUBRESOURCE_TILING* pSubresourceTilingsForNonPackedMips)
    {
        redirect->GetResourceTiling(pTiledResource, pNumTilesForEntireResource, pPackedMipDesc, pStandardTileShapeForNonPackedMips, pNumSubresourceTilings, FirstSubresourceTilingToGet, pSubresourceTilingsForNonPackedMips);
    }

#if defined(_MSC_VER) || !defined(_WIN32)
    virtual LUID STDMETHODCALLTYPE GetAdapterLuid(void)
    {
        return redirect->GetAdapterLuid();
    }
#else
    virtual LUID* STDMETHODCALLTYPE GetAdapterLuid(
        LUID* RetVal)
    {
        return redirect->GetAdapterLuid(RetVal);
    }
#endif

    //

    virtual HRESULT STDMETHODCALLTYPE CreatePipelineLibrary(
        _In_reads_(BlobLength)  const void* pLibraryBlob,
        SIZE_T BlobLength,
        REFIID riid,
        _COM_Outptr_  void** ppPipelineLibrary)
    {
        return redirect->CreatePipelineLibrary(pLibraryBlob, BlobLength, riid, ppPipelineLibrary);
    }

    virtual HRESULT STDMETHODCALLTYPE SetEventOnMultipleFenceCompletion(
        _In_reads_(NumFences)  ID3D12Fence* const* ppFences,
        _In_reads_(NumFences)  const UINT64* pFenceValues,
        UINT NumFences,
        D3D12_MULTIPLE_FENCE_WAIT_FLAGS Flags,
        HANDLE hEvent)
    {
        return redirect->SetEventOnMultipleFenceCompletion(ppFences, pFenceValues, NumFences, Flags, hEvent);
    }

    virtual HRESULT STDMETHODCALLTYPE SetResidencyPriority(
        UINT NumObjects,
        _In_reads_(NumObjects)  ID3D12Pageable* const* ppObjects,
        _In_reads_(NumObjects)  const D3D12_RESIDENCY_PRIORITY* pPriorities)
    {
        return redirect->SetResidencyPriority(NumObjects, ppObjects, pPriorities);
    }

    // 

    virtual HRESULT STDMETHODCALLTYPE CreatePipelineState(
        const D3D12_PIPELINE_STATE_STREAM_DESC* pDesc,
        REFIID riid,
        _COM_Outptr_  void** ppPipelineState)
    {
        return redirect->CreatePipelineState(pDesc, riid, ppPipelineState);
    }
};

