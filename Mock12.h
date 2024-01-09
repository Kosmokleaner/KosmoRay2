#pragma once
#include <d3d12.h>
#include <stdio.h>
#include <assert.h>
#include <algorithm>    // std::min

#include <wrl.h>
using namespace Microsoft::WRL;

#undef min

// not yet wrapped:
// Mock12Device2 : ID3D12InfoQueue
// Mock12Device2 : ID3D12Device5
// 
// TODO ID3D12CommandAllocator
// TODO ID3D12PipelineState
// DONE ID3D12Resource
// TODO ID3D12Heap
// TODO ID3D12DeviceChild
// TODO ID3D12Pageable
// TODO ID3D12RootSignature
// TODO ID3D12Fence
// DONE ID3D12CommandList
// DONE ID3D12CommandQueue

void Mock12Test();

inline ID3D12CommandList* castDown(ID3D12CommandList* res);
inline ID3D12Resource* castDown(ID3D12Resource* res);

#define IMPLEMENT_IUNKNOWN \
    ULONG m_dwRef = 1; \
    virtual ULONG STDMETHODCALLTYPE AddRef() \
    { return ++m_dwRef; } \
    virtual ULONG STDMETHODCALLTYPE Release() \
    { if (--m_dwRef == 0) { delete this; return 0; } \
        return m_dwRef; \
    }

struct Mock12CommandQueue : public ID3D12CommandQueue
{
    IMPLEMENT_IUNKNOWN

    ComPtr<ID3D12CommandQueue> redirect;

    Mock12CommandQueue(ID3D12CommandQueue* inRedirect)
    {
        redirect = inRedirect;
    }

    BEGIN_INTERFACE virtual HRESULT STDMETHODCALLTYPE QueryInterface(
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject)
    {
        return redirect->QueryInterface(riid, ppvObject);
    }

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

    // ID3D12DeviceChild

    virtual HRESULT STDMETHODCALLTYPE GetDevice(
        REFIID riid,
        _COM_Outptr_opt_  void** ppvDevice)
    {
        return redirect->GetDevice(riid, ppvDevice);
    }

    //

    virtual void STDMETHODCALLTYPE UpdateTileMappings(
        _In_  ID3D12Resource* pResource,
        UINT NumResourceRegions,
        _In_reads_opt_(NumResourceRegions)  const D3D12_TILED_RESOURCE_COORDINATE* pResourceRegionStartCoordinates,
        _In_reads_opt_(NumResourceRegions)  const D3D12_TILE_REGION_SIZE* pResourceRegionSizes,
        _In_opt_  ID3D12Heap* pHeap,
        UINT NumRanges,
        _In_reads_opt_(NumRanges)  const D3D12_TILE_RANGE_FLAGS* pRangeFlags,
        _In_reads_opt_(NumRanges)  const UINT* pHeapRangeStartOffsets,
        _In_reads_opt_(NumRanges)  const UINT* pRangeTileCounts,
        D3D12_TILE_MAPPING_FLAGS Flags)
    {
        redirect->UpdateTileMappings(pResource, NumResourceRegions, pResourceRegionStartCoordinates, pResourceRegionSizes, pHeap, NumRanges, pRangeFlags, pHeapRangeStartOffsets, pRangeTileCounts, Flags);
    }

    virtual void STDMETHODCALLTYPE CopyTileMappings(
        _In_  ID3D12Resource* pDstResource,
        _In_  const D3D12_TILED_RESOURCE_COORDINATE* pDstRegionStartCoordinate,
        _In_  ID3D12Resource* pSrcResource,
        _In_  const D3D12_TILED_RESOURCE_COORDINATE* pSrcRegionStartCoordinate,
        _In_  const D3D12_TILE_REGION_SIZE* pRegionSize,
        D3D12_TILE_MAPPING_FLAGS Flags)
    {
        redirect->CopyTileMappings(pDstResource, pDstRegionStartCoordinate, pSrcResource, pSrcRegionStartCoordinate, pRegionSize, Flags);
    }

    virtual void STDMETHODCALLTYPE ExecuteCommandLists(
        _In_  UINT NumCommandLists,
        _In_reads_(NumCommandLists)  ID3D12CommandList* const* ppCommandLists)
    {
        ID3D12CommandList* tmp[128];
        assert(NumCommandLists <= 128);

        for(UINT i = 0; i < NumCommandLists; ++i)
            tmp[i] = castDown(ppCommandLists[i]);

        redirect->ExecuteCommandLists(NumCommandLists, tmp);
    }

    virtual void STDMETHODCALLTYPE SetMarker(
        UINT Metadata,
        _In_reads_bytes_opt_(Size)  const void* pData,
        UINT Size)
    {
        redirect->SetMarker(Metadata, pData, Size);
    }

    virtual void STDMETHODCALLTYPE BeginEvent(
        UINT Metadata,
        _In_reads_bytes_opt_(Size)  const void* pData,
        UINT Size)
    {
        redirect->BeginEvent(Metadata, pData, Size);
    }

    virtual void STDMETHODCALLTYPE EndEvent(void)
    {
        redirect->EndEvent();
    }

    virtual HRESULT STDMETHODCALLTYPE Signal(
        ID3D12Fence* pFence,
        UINT64 Value)
    {
        return redirect->Signal(pFence, Value);
    }

    virtual HRESULT STDMETHODCALLTYPE Wait(
        ID3D12Fence* pFence,
        UINT64 Value)
    {
        return redirect->Wait(pFence, Value);
    }

    virtual HRESULT STDMETHODCALLTYPE GetTimestampFrequency(
        _Out_  UINT64* pFrequency)
    {
        return redirect->GetTimestampFrequency(pFrequency);
    }

    virtual HRESULT STDMETHODCALLTYPE GetClockCalibration(
        _Out_  UINT64* pGpuTimestamp,
        _Out_  UINT64* pCpuTimestamp)
    {
        return redirect->GetClockCalibration(pGpuTimestamp, pCpuTimestamp);
    }

#if defined(_MSC_VER) || !defined(_WIN32)
    virtual D3D12_COMMAND_QUEUE_DESC STDMETHODCALLTYPE GetDesc(void)
    {
        return redirect->GetDesc();
    }
#else
    virtual D3D12_COMMAND_QUEUE_DESC* STDMETHODCALLTYPE GetDesc(
        D3D12_COMMAND_QUEUE_DESC* RetVal)
    {
        return redirect->GetDesc(RetVal);
    }
#endif
};

struct Mock12CommandList : public ID3D12GraphicsCommandList4
{
    IMPLEMENT_IUNKNOWN

    // {1B12B3DC-3387-46A3-9457-8536BE6289B4}
    static const GUID guid;

    ComPtr<ID3D12GraphicsCommandList4> redirect;

    Mock12CommandList(ID3D12GraphicsCommandList4* inRedirect)
    {
        redirect = inRedirect;
    }

    BEGIN_INTERFACE virtual HRESULT STDMETHODCALLTYPE QueryInterface(
        /* [in] */ REFIID riid,
        /* [iid_is][out] */ _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject)
    {
        if (riid == guid)
        {
            *ppvObject = redirect.Get();
            return S_OK;
        }

        return redirect->QueryInterface(riid, ppvObject);
    }

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

    // ID3D12DeviceChild

    virtual HRESULT STDMETHODCALLTYPE GetDevice(
        REFIID riid,
        _COM_Outptr_opt_  void** ppvDevice)
    {
        return redirect->GetDevice(riid, ppvDevice);
    }

    // ID3D12CommandList

    virtual D3D12_COMMAND_LIST_TYPE STDMETHODCALLTYPE GetType(void)
    {
        return redirect->GetType();
    }

    // ID3D12GraphicsCommandList

    virtual HRESULT STDMETHODCALLTYPE Close(void)
    {
        return redirect->Close();
    }

    virtual HRESULT STDMETHODCALLTYPE Reset(
        _In_  ID3D12CommandAllocator* pAllocator,
        _In_opt_  ID3D12PipelineState* pInitialState) 
    {
        return redirect->Reset(pAllocator, pInitialState);
    }

    virtual void STDMETHODCALLTYPE ClearState(
        _In_opt_  ID3D12PipelineState* pPipelineState)
    {
        return redirect->ClearState(pPipelineState);
    }

    virtual void STDMETHODCALLTYPE DrawInstanced(
        _In_  UINT VertexCountPerInstance,
        _In_  UINT InstanceCount,
        _In_  UINT StartVertexLocation,
        _In_  UINT StartInstanceLocation)
    {
        return redirect->DrawInstanced(VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);
    }

    virtual void STDMETHODCALLTYPE DrawIndexedInstanced(
        _In_  UINT IndexCountPerInstance,
        _In_  UINT InstanceCount,
        _In_  UINT StartIndexLocation,
        _In_  INT BaseVertexLocation,
        _In_  UINT StartInstanceLocation)
    {
        redirect->DrawIndexedInstanced(IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
    }

    virtual void STDMETHODCALLTYPE Dispatch(
        _In_  UINT ThreadGroupCountX,
        _In_  UINT ThreadGroupCountY,
        _In_  UINT ThreadGroupCountZ)
    {
        redirect->Dispatch(ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);
    }

    virtual void STDMETHODCALLTYPE CopyBufferRegion(
        _In_  ID3D12Resource* pDstBuffer,
        UINT64 DstOffset,
        _In_  ID3D12Resource* pSrcBuffer,
        UINT64 SrcOffset,
        UINT64 NumBytes)
    {
        redirect->CopyBufferRegion(pDstBuffer, DstOffset, pSrcBuffer, SrcOffset, NumBytes);
    }

    virtual void STDMETHODCALLTYPE CopyTextureRegion(
        _In_  const D3D12_TEXTURE_COPY_LOCATION* pDst,
        UINT DstX,
        UINT DstY,
        UINT DstZ,
        _In_  const D3D12_TEXTURE_COPY_LOCATION* pSrc,
        _In_opt_  const D3D12_BOX* pSrcBox)
    {
        redirect->CopyTextureRegion(pDst, DstX, DstY, DstZ, pSrc, pSrcBox);
    }

    virtual void STDMETHODCALLTYPE CopyResource(
        _In_  ID3D12Resource* pDstResource,
        _In_  ID3D12Resource* pSrcResource)
    {
        pDstResource = castDown(pDstResource);
        pSrcResource = castDown(pSrcResource);

        redirect->CopyResource(pDstResource, pSrcResource);
    }

    virtual void STDMETHODCALLTYPE CopyTiles(
        _In_  ID3D12Resource* pTiledResource,
        _In_  const D3D12_TILED_RESOURCE_COORDINATE* pTileRegionStartCoordinate,
        _In_  const D3D12_TILE_REGION_SIZE* pTileRegionSize,
        _In_  ID3D12Resource* pBuffer,
        UINT64 BufferStartOffsetInBytes,
        D3D12_TILE_COPY_FLAGS Flags)
    {
        redirect->CopyTiles(pTiledResource, pTileRegionStartCoordinate, pTileRegionSize, pBuffer, BufferStartOffsetInBytes, Flags);
    }

    virtual void STDMETHODCALLTYPE ResolveSubresource(
        _In_  ID3D12Resource* pDstResource,
        _In_  UINT DstSubresource,
        _In_  ID3D12Resource* pSrcResource,
        _In_  UINT SrcSubresource,
        _In_  DXGI_FORMAT Format)
    {
        redirect->ResolveSubresource(pDstResource, DstSubresource, pSrcResource, SrcSubresource, Format);
    }

    virtual void STDMETHODCALLTYPE IASetPrimitiveTopology(
        _In_  D3D12_PRIMITIVE_TOPOLOGY PrimitiveTopology)
    {
        redirect->IASetPrimitiveTopology(PrimitiveTopology);
    }

    virtual void STDMETHODCALLTYPE RSSetViewports(
        _In_range_(0, D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE)  UINT NumViewports,
        _In_reads_(NumViewports)  const D3D12_VIEWPORT* pViewports)
    {
        redirect->RSSetViewports(NumViewports, pViewports);
    }

    virtual void STDMETHODCALLTYPE RSSetScissorRects(
        _In_range_(0, D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE)  UINT NumRects,
        _In_reads_(NumRects)  const D3D12_RECT* pRects)
    {
        redirect->RSSetScissorRects(NumRects, pRects);
    }

    virtual void STDMETHODCALLTYPE OMSetBlendFactor(
        _In_reads_opt_(4)  const FLOAT BlendFactor[4])
    {
        redirect->OMSetBlendFactor(BlendFactor);
    }

    virtual void STDMETHODCALLTYPE OMSetStencilRef(
        _In_  UINT StencilRef)
    {
        redirect->OMSetStencilRef(StencilRef);
    }

    virtual void STDMETHODCALLTYPE SetPipelineState(
        _In_  ID3D12PipelineState* pPipelineState)
    {
        redirect->SetPipelineState(pPipelineState);
    }

    virtual void STDMETHODCALLTYPE ResourceBarrier(
        _In_  UINT NumBarriers,
        _In_reads_(NumBarriers)  const D3D12_RESOURCE_BARRIER* pBarriers)
    {
        D3D12_RESOURCE_BARRIER tmp[256];
        assert(NumBarriers < 256);
        memcpy(tmp, pBarriers, std::min(NumBarriers, (UINT)256) * sizeof(D3D12_RESOURCE_BARRIER));

        for(UINT i = 0; i < NumBarriers; ++i)
            tmp[i].Transition.pResource = castDown(tmp[i].Transition.pResource);

        redirect->ResourceBarrier(NumBarriers, tmp);
    }

    virtual void STDMETHODCALLTYPE ExecuteBundle(
        _In_  ID3D12GraphicsCommandList* pCommandList)
    {
        redirect->ExecuteBundle(pCommandList);
    }

    virtual void STDMETHODCALLTYPE SetDescriptorHeaps(
        _In_  UINT NumDescriptorHeaps,
        _In_reads_(NumDescriptorHeaps)  ID3D12DescriptorHeap* const* ppDescriptorHeaps)
    {
        redirect->SetDescriptorHeaps(NumDescriptorHeaps, ppDescriptorHeaps);
    }

    virtual void STDMETHODCALLTYPE SetComputeRootSignature(
        _In_opt_  ID3D12RootSignature* pRootSignature)
    {
        redirect->SetComputeRootSignature(pRootSignature);
    }

    virtual void STDMETHODCALLTYPE SetGraphicsRootSignature(
        _In_opt_  ID3D12RootSignature* pRootSignature)
    {
        redirect->SetGraphicsRootSignature(pRootSignature);
    }

    virtual void STDMETHODCALLTYPE SetComputeRootDescriptorTable(
        _In_  UINT RootParameterIndex,
        _In_  D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor)
    {
        redirect->SetComputeRootDescriptorTable(RootParameterIndex, BaseDescriptor);
    }

    virtual void STDMETHODCALLTYPE SetGraphicsRootDescriptorTable(
        _In_  UINT RootParameterIndex,
        _In_  D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor)
    {
        redirect->SetGraphicsRootDescriptorTable(RootParameterIndex, BaseDescriptor);
    }

    virtual void STDMETHODCALLTYPE SetComputeRoot32BitConstant(
        _In_  UINT RootParameterIndex,
        _In_  UINT SrcData,
        _In_  UINT DestOffsetIn32BitValues)
    {
        redirect->SetComputeRoot32BitConstant(RootParameterIndex, SrcData, DestOffsetIn32BitValues);
    }

    virtual void STDMETHODCALLTYPE SetGraphicsRoot32BitConstant(
        _In_  UINT RootParameterIndex,
        _In_  UINT SrcData,
        _In_  UINT DestOffsetIn32BitValues)
    {
        redirect->SetGraphicsRoot32BitConstant(RootParameterIndex, SrcData, DestOffsetIn32BitValues);
    }

    virtual void STDMETHODCALLTYPE SetComputeRoot32BitConstants(
        _In_  UINT RootParameterIndex,
        _In_  UINT Num32BitValuesToSet,
        _In_reads_(Num32BitValuesToSet * sizeof(UINT))  const void* pSrcData,
        _In_  UINT DestOffsetIn32BitValues)
    {
        redirect->SetComputeRoot32BitConstants(RootParameterIndex, Num32BitValuesToSet, pSrcData, DestOffsetIn32BitValues);
    }

    virtual void STDMETHODCALLTYPE SetGraphicsRoot32BitConstants(
        _In_  UINT RootParameterIndex,
        _In_  UINT Num32BitValuesToSet,
        _In_reads_(Num32BitValuesToSet * sizeof(UINT))  const void* pSrcData,
        _In_  UINT DestOffsetIn32BitValues)
    {
        redirect->SetGraphicsRoot32BitConstants(RootParameterIndex, Num32BitValuesToSet, pSrcData, DestOffsetIn32BitValues);
    }

    virtual void STDMETHODCALLTYPE SetComputeRootConstantBufferView(
        _In_  UINT RootParameterIndex,
        _In_  D3D12_GPU_VIRTUAL_ADDRESS BufferLocation)
    {
        redirect->SetComputeRootConstantBufferView(RootParameterIndex, BufferLocation);
    }

    virtual void STDMETHODCALLTYPE SetGraphicsRootConstantBufferView(
        _In_  UINT RootParameterIndex,
        _In_  D3D12_GPU_VIRTUAL_ADDRESS BufferLocation)
    {
        redirect->SetGraphicsRootConstantBufferView(RootParameterIndex, BufferLocation);
    }

    virtual void STDMETHODCALLTYPE SetComputeRootShaderResourceView(
        _In_  UINT RootParameterIndex,
        _In_  D3D12_GPU_VIRTUAL_ADDRESS BufferLocation)
    {
        redirect->SetComputeRootShaderResourceView(RootParameterIndex, BufferLocation);
    }

    virtual void STDMETHODCALLTYPE SetGraphicsRootShaderResourceView(
        _In_  UINT RootParameterIndex,
        _In_  D3D12_GPU_VIRTUAL_ADDRESS BufferLocation)
    {
        redirect->SetGraphicsRootShaderResourceView(RootParameterIndex, BufferLocation);
    }

    virtual void STDMETHODCALLTYPE SetComputeRootUnorderedAccessView(
        _In_  UINT RootParameterIndex,
        _In_  D3D12_GPU_VIRTUAL_ADDRESS BufferLocation)
    {
        redirect->SetComputeRootUnorderedAccessView(RootParameterIndex, BufferLocation);
    }

    virtual void STDMETHODCALLTYPE SetGraphicsRootUnorderedAccessView(
        _In_  UINT RootParameterIndex,
        _In_  D3D12_GPU_VIRTUAL_ADDRESS BufferLocation)
    {
        redirect->SetGraphicsRootUnorderedAccessView(RootParameterIndex, BufferLocation);
    }

    virtual void STDMETHODCALLTYPE IASetIndexBuffer(
        _In_opt_  const D3D12_INDEX_BUFFER_VIEW* pView)
    {
        redirect->IASetIndexBuffer(pView);
    }

    virtual void STDMETHODCALLTYPE IASetVertexBuffers(
        _In_  UINT StartSlot,
        _In_  UINT NumViews,
        _In_reads_opt_(NumViews)  const D3D12_VERTEX_BUFFER_VIEW* pViews)
    {
        redirect->IASetVertexBuffers(StartSlot, NumViews, pViews);
    }

    virtual void STDMETHODCALLTYPE SOSetTargets(
        _In_  UINT StartSlot,
        _In_  UINT NumViews,
        _In_reads_opt_(NumViews)  const D3D12_STREAM_OUTPUT_BUFFER_VIEW* pViews)
    {
        redirect->SOSetTargets(StartSlot, NumViews, pViews);
    }

    virtual void STDMETHODCALLTYPE OMSetRenderTargets(
        _In_  UINT NumRenderTargetDescriptors,
        _In_opt_  const D3D12_CPU_DESCRIPTOR_HANDLE* pRenderTargetDescriptors,
        _In_  BOOL RTsSingleHandleToDescriptorRange,
        _In_opt_  const D3D12_CPU_DESCRIPTOR_HANDLE* pDepthStencilDescriptor)
    {
        redirect->OMSetRenderTargets(NumRenderTargetDescriptors, pRenderTargetDescriptors, RTsSingleHandleToDescriptorRange, pDepthStencilDescriptor);
    }

    virtual void STDMETHODCALLTYPE ClearDepthStencilView(
        _In_  D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView,
        _In_  D3D12_CLEAR_FLAGS ClearFlags,
        _In_  FLOAT Depth,
        _In_  UINT8 Stencil,
        _In_  UINT NumRects,
        _In_reads_(NumRects)  const D3D12_RECT* pRects)
    {
        redirect->ClearDepthStencilView(DepthStencilView, ClearFlags, Depth, Stencil, NumRects, pRects);
    }

    virtual void STDMETHODCALLTYPE ClearRenderTargetView(
        _In_  D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetView,
        _In_  const FLOAT ColorRGBA[4],
        _In_  UINT NumRects,
        _In_reads_(NumRects)  const D3D12_RECT* pRects)
    {
        redirect->ClearRenderTargetView(RenderTargetView, ColorRGBA, NumRects, pRects);
    }

    virtual void STDMETHODCALLTYPE ClearUnorderedAccessViewUint(
        _In_  D3D12_GPU_DESCRIPTOR_HANDLE ViewGPUHandleInCurrentHeap,
        _In_  D3D12_CPU_DESCRIPTOR_HANDLE ViewCPUHandle,
        _In_  ID3D12Resource* pResource,
        _In_  const UINT Values[4],
        _In_  UINT NumRects,
        _In_reads_(NumRects)  const D3D12_RECT* pRects)
    {
        redirect->ClearUnorderedAccessViewUint(ViewGPUHandleInCurrentHeap, ViewCPUHandle, pResource, Values, NumRects, pRects);
    }

    virtual void STDMETHODCALLTYPE ClearUnorderedAccessViewFloat(
        _In_  D3D12_GPU_DESCRIPTOR_HANDLE ViewGPUHandleInCurrentHeap,
        _In_  D3D12_CPU_DESCRIPTOR_HANDLE ViewCPUHandle,
        _In_  ID3D12Resource* pResource,
        _In_  const FLOAT Values[4],
        _In_  UINT NumRects,
        _In_reads_(NumRects)  const D3D12_RECT* pRects)
    {
        redirect->ClearUnorderedAccessViewFloat(ViewGPUHandleInCurrentHeap, ViewCPUHandle, pResource, Values, NumRects, pRects);
    }

    virtual void STDMETHODCALLTYPE DiscardResource(
        _In_  ID3D12Resource* pResource,
        _In_opt_  const D3D12_DISCARD_REGION* pRegion)
    {
        redirect->DiscardResource(pResource, pRegion);
    }

    virtual void STDMETHODCALLTYPE BeginQuery(
        _In_  ID3D12QueryHeap* pQueryHeap,
        _In_  D3D12_QUERY_TYPE Type,
        _In_  UINT Index)
    {
        redirect->BeginQuery(pQueryHeap, Type, Index);
    }

    virtual void STDMETHODCALLTYPE EndQuery(
        _In_  ID3D12QueryHeap* pQueryHeap,
        _In_  D3D12_QUERY_TYPE Type,
        _In_  UINT Index)
    {
        redirect->EndQuery(pQueryHeap, Type, Index);
    }

    virtual void STDMETHODCALLTYPE ResolveQueryData(
        _In_  ID3D12QueryHeap* pQueryHeap,
        _In_  D3D12_QUERY_TYPE Type,
        _In_  UINT StartIndex,
        _In_  UINT NumQueries,
        _In_  ID3D12Resource* pDestinationBuffer,
        _In_  UINT64 AlignedDestinationBufferOffset)
    {
        redirect->ResolveQueryData(pQueryHeap, Type, StartIndex, NumQueries, pDestinationBuffer, AlignedDestinationBufferOffset);
    }

    virtual void STDMETHODCALLTYPE SetPredication(
        _In_opt_  ID3D12Resource* pBuffer,
        _In_  UINT64 AlignedBufferOffset,
        _In_  D3D12_PREDICATION_OP Operation)
    {
        redirect->SetPredication(pBuffer, AlignedBufferOffset, Operation);
    }

    virtual void STDMETHODCALLTYPE SetMarker(
        UINT Metadata,
        _In_reads_bytes_opt_(Size)  const void* pData,
        UINT Size)
    {
        redirect->SetMarker(Metadata, pData, Size);
    }

    virtual void STDMETHODCALLTYPE BeginEvent(
        UINT Metadata,
        _In_reads_bytes_opt_(Size)  const void* pData,
        UINT Size)
    {
        redirect->BeginEvent(Metadata, pData, Size);
    }

    virtual void STDMETHODCALLTYPE EndEvent(void)
    {
        redirect->EndEvent();
    }

    virtual void STDMETHODCALLTYPE ExecuteIndirect(
        _In_  ID3D12CommandSignature* pCommandSignature,
        _In_  UINT MaxCommandCount,
        _In_  ID3D12Resource* pArgumentBuffer,
        _In_  UINT64 ArgumentBufferOffset,
        _In_opt_  ID3D12Resource* pCountBuffer,
        _In_  UINT64 CountBufferOffset)
    {
        redirect->ExecuteIndirect(pCommandSignature, MaxCommandCount, pArgumentBuffer, ArgumentBufferOffset, pCountBuffer, CountBufferOffset);
    }

    // ID3D12GraphicsCommandList1

    virtual void STDMETHODCALLTYPE AtomicCopyBufferUINT(
        _In_  ID3D12Resource* pDstBuffer,
        UINT64 DstOffset,
        _In_  ID3D12Resource* pSrcBuffer,
        UINT64 SrcOffset,
        UINT Dependencies,
        _In_reads_(Dependencies)  ID3D12Resource* const* ppDependentResources,
        _In_reads_(Dependencies)  const D3D12_SUBRESOURCE_RANGE_UINT64* pDependentSubresourceRanges)
    {
        redirect->AtomicCopyBufferUINT(pDstBuffer, DstOffset, pSrcBuffer, SrcOffset, Dependencies, ppDependentResources, pDependentSubresourceRanges);
    }

    virtual void STDMETHODCALLTYPE AtomicCopyBufferUINT64(
        _In_  ID3D12Resource* pDstBuffer,
        UINT64 DstOffset,
        _In_  ID3D12Resource* pSrcBuffer,
        UINT64 SrcOffset,
        UINT Dependencies,
        _In_reads_(Dependencies)  ID3D12Resource* const* ppDependentResources,
        _In_reads_(Dependencies)  const D3D12_SUBRESOURCE_RANGE_UINT64* pDependentSubresourceRanges)
    {
        redirect->AtomicCopyBufferUINT64(pDstBuffer, DstOffset, pSrcBuffer, SrcOffset, Dependencies, ppDependentResources, pDependentSubresourceRanges);
    }

    virtual void STDMETHODCALLTYPE OMSetDepthBounds(
        _In_  FLOAT Min,
        _In_  FLOAT Max)
    {
        redirect->OMSetDepthBounds(Min, Max);
    }

    virtual void STDMETHODCALLTYPE SetSamplePositions(
        _In_  UINT NumSamplesPerPixel,
        _In_  UINT NumPixels,
        _In_reads_(NumSamplesPerPixel* NumPixels)  D3D12_SAMPLE_POSITION* pSamplePositions)
    {
        redirect->SetSamplePositions(NumSamplesPerPixel, NumPixels, pSamplePositions);
    }

    virtual void STDMETHODCALLTYPE ResolveSubresourceRegion(
        _In_  ID3D12Resource* pDstResource,
        _In_  UINT DstSubresource,
        _In_  UINT DstX,
        _In_  UINT DstY,
        _In_  ID3D12Resource* pSrcResource,
        _In_  UINT SrcSubresource,
        _In_opt_  D3D12_RECT* pSrcRect,
        _In_  DXGI_FORMAT Format,
        _In_  D3D12_RESOLVE_MODE ResolveMode)
    {
        redirect->ResolveSubresourceRegion(pDstResource, DstSubresource, DstX, DstY, pSrcResource, SrcSubresource, pSrcRect, Format, ResolveMode);
    }

    virtual void STDMETHODCALLTYPE SetViewInstanceMask(
        _In_  UINT Mask)
    {
        redirect->SetViewInstanceMask(Mask);
    }

    // ID3D12GraphicsCommandList2

    virtual void STDMETHODCALLTYPE WriteBufferImmediate(
        UINT Count,
        _In_reads_(Count)  const D3D12_WRITEBUFFERIMMEDIATE_PARAMETER* pParams,
        _In_reads_opt_(Count)  const D3D12_WRITEBUFFERIMMEDIATE_MODE* pModes)
    {
        redirect->WriteBufferImmediate(Count, pParams, pModes);
    }

    // ID3D12GraphicsCommandList3

    virtual void STDMETHODCALLTYPE SetProtectedResourceSession(
        _In_opt_  ID3D12ProtectedResourceSession* pProtectedResourceSession)
    {
        redirect->SetProtectedResourceSession(pProtectedResourceSession);
    }

    // ID3D12GraphicsCommandList4

    virtual void STDMETHODCALLTYPE BeginRenderPass(
        _In_  UINT NumRenderTargets,
        _In_reads_opt_(NumRenderTargets)  const D3D12_RENDER_PASS_RENDER_TARGET_DESC* pRenderTargets,
        _In_opt_  const D3D12_RENDER_PASS_DEPTH_STENCIL_DESC* pDepthStencil,
        D3D12_RENDER_PASS_FLAGS Flags)
    {
        redirect->BeginRenderPass(NumRenderTargets, pRenderTargets, pDepthStencil, Flags);
    }

    virtual void STDMETHODCALLTYPE EndRenderPass(void)
    {
        redirect->EndRenderPass();
    }

    virtual void STDMETHODCALLTYPE InitializeMetaCommand(
        _In_  ID3D12MetaCommand* pMetaCommand,
        _In_reads_bytes_opt_(InitializationParametersDataSizeInBytes)  const void* pInitializationParametersData,
        _In_  SIZE_T InitializationParametersDataSizeInBytes)
    {
        redirect->InitializeMetaCommand(pMetaCommand, pInitializationParametersData, InitializationParametersDataSizeInBytes);
    }

    virtual void STDMETHODCALLTYPE ExecuteMetaCommand(
        _In_  ID3D12MetaCommand* pMetaCommand,
        _In_reads_bytes_opt_(ExecutionParametersDataSizeInBytes)  const void* pExecutionParametersData,
        _In_  SIZE_T ExecutionParametersDataSizeInBytes)
    {
        redirect->ExecuteMetaCommand(pMetaCommand, pExecutionParametersData, ExecutionParametersDataSizeInBytes);
    }

    virtual void STDMETHODCALLTYPE BuildRaytracingAccelerationStructure(
        _In_  const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC* pDesc,
        _In_  UINT NumPostbuildInfoDescs,
        _In_reads_opt_(NumPostbuildInfoDescs)  const D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_DESC* pPostbuildInfoDescs)
    {
        redirect->BuildRaytracingAccelerationStructure(pDesc, NumPostbuildInfoDescs, pPostbuildInfoDescs);
    }

    virtual void STDMETHODCALLTYPE EmitRaytracingAccelerationStructurePostbuildInfo(
        _In_  const D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_DESC* pDesc,
        _In_  UINT NumSourceAccelerationStructures,
        _In_reads_(NumSourceAccelerationStructures)  const D3D12_GPU_VIRTUAL_ADDRESS* pSourceAccelerationStructureData)
    {
        redirect->EmitRaytracingAccelerationStructurePostbuildInfo(pDesc, NumSourceAccelerationStructures, pSourceAccelerationStructureData);
    }

    virtual void STDMETHODCALLTYPE CopyRaytracingAccelerationStructure(
        _In_  D3D12_GPU_VIRTUAL_ADDRESS DestAccelerationStructureData,
        _In_  D3D12_GPU_VIRTUAL_ADDRESS SourceAccelerationStructureData,
        _In_  D3D12_RAYTRACING_ACCELERATION_STRUCTURE_COPY_MODE Mode)
    {
        redirect->CopyRaytracingAccelerationStructure(DestAccelerationStructureData, SourceAccelerationStructureData, Mode);
    }

    virtual void STDMETHODCALLTYPE SetPipelineState1(
        _In_  ID3D12StateObject* pStateObject)
    {
        redirect->SetPipelineState1(pStateObject);
    }

    virtual void STDMETHODCALLTYPE DispatchRays(
        _In_  const D3D12_DISPATCH_RAYS_DESC* pDesc)
    {
        redirect->DispatchRays(pDesc);
    }
};


struct Mock12Resource : public ID3D12Resource
{
    IMPLEMENT_IUNKNOWN

    // {E4D1943F-0E48-4B63-8017-E9F8FC650C8C}
    static const GUID guid;

    ComPtr<ID3D12Resource> redirect;

    Mock12Resource(ID3D12Resource* inRedirect)
    {
        redirect = inRedirect;
    }

    BEGIN_INTERFACE
        virtual HRESULT STDMETHODCALLTYPE QueryInterface(
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject)
    {
        if(riid == guid)
        {
            *ppvObject = redirect.Get();
            return S_OK;
        }

        return redirect->QueryInterface(riid, ppvObject);
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
        char str[1024];
        sprintf_s(str, sizeof(str), "SetName %p: '%ls'\n", this, Name);
        OutputDebugStringA(str);
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

inline ID3D12Resource* castDown(ID3D12Resource* res)
{
    if (res)
    {
        ID3D12Resource* tmp = nullptr;
        if (res->QueryInterface(Mock12Resource::guid, (void**)&tmp) == S_OK)
        {
            return tmp;
        }
    }

    return res;
}

inline ID3D12CommandList* castDown(ID3D12CommandList* res)
{
    if (res)
    {
        ID3D12CommandList* tmp = nullptr;
        if (res->QueryInterface(Mock12CommandList::guid, (void**)&tmp) == S_OK)
        {
            return tmp;
        }
    }

    return res;
}

// ----------------------------------------------------------------------------

struct Mock12Device2 : public ID3D12Device2
{
    IMPLEMENT_IUNKNOWN

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
        HRESULT ret = redirect->CreateCommandQueue(pDesc, riid, ppCommandQueue);

        IUnknown* unk = *(IUnknown**)ppCommandQueue;
        ID3D12CommandQueue* res = nullptr;
        unk->QueryInterface(__uuidof(ID3D12CommandQueue), (void**)&res);
        if (res)
        {
            Mock12CommandQueue* mock = new Mock12CommandQueue(res);
            *ppCommandQueue = (void*)mock;
        }

        return ret;
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
        HRESULT ret = redirect->CreateCommandList(nodeMask, type, pCommandAllocator, pInitialState, riid, ppCommandList);

        IUnknown* unk = *(IUnknown**)ppCommandList;
        ID3D12GraphicsCommandList4* res = nullptr;
        unk->QueryInterface(__uuidof(ID3D12GraphicsCommandList4), (void**)&res);
        if (res)
        {
            Mock12CommandList* mock = new Mock12CommandList(res);
            *ppCommandList = (void*)mock;
        }

        return ret;
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
        pResource = castDown(pResource);
        redirect->CreateShaderResourceView(pResource, pDesc, DestDescriptor);
    }

    virtual void STDMETHODCALLTYPE CreateUnorderedAccessView(
        _In_opt_  ID3D12Resource* pResource,
        _In_opt_  ID3D12Resource* pCounterResource,
        _In_opt_  const D3D12_UNORDERED_ACCESS_VIEW_DESC* pDesc,
        _In_  D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor)
    {
        pResource = castDown(pResource);
        redirect->CreateUnorderedAccessView(pResource, pCounterResource, pDesc, DestDescriptor);
    }

    virtual void STDMETHODCALLTYPE CreateRenderTargetView(
        _In_opt_  ID3D12Resource* pResource,
        _In_opt_  const D3D12_RENDER_TARGET_VIEW_DESC* pDesc,
        _In_  D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor)
    {
        pResource = castDown(pResource);
        redirect->CreateRenderTargetView(pResource, pDesc, DestDescriptor);
    }

    virtual void STDMETHODCALLTYPE CreateDepthStencilView(
        _In_opt_  ID3D12Resource* pResource,
        _In_opt_  const D3D12_DEPTH_STENCIL_VIEW_DESC* pDesc,
        _In_  D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor)
    {
        pResource = castDown(pResource);
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
        HRESULT ret = redirect->CreateCommittedResource(pHeapProperties, HeapFlags, pDesc, InitialResourceState, pOptimizedClearValue, riidResource, ppvResource);

        IUnknown* unk = *(IUnknown**)ppvResource;
        ID3D12Resource* res = nullptr;
        unk->QueryInterface(__uuidof(ID3D12Resource), (void**)&res);
        if (res)
        {
            Mock12Resource *mock = new Mock12Resource(res);
            *ppvResource = (void*)mock;
        }

        return ret;
    }

    virtual HRESULT STDMETHODCALLTYPE CreateHeap(
        _In_  const D3D12_HEAP_DESC* pDesc,
        REFIID riid,
        _COM_Outptr_opt_  void** ppvHeap)
    {
        HRESULT ret = redirect->CreateHeap(pDesc, riid, ppvHeap);

        IUnknown* unk = *(IUnknown**)ppvHeap;
        ID3D12Resource* res = nullptr;
        unk->QueryInterface(__uuidof(ID3D12Resource), (void**)&res);
        if (res)
            __debugbreak();

        return ret;
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
        HRESULT ret = redirect->CreatePlacedResource(pHeap, HeapOffset, pDesc, InitialState, pOptimizedClearValue, riid, ppvResource);

        IUnknown* unk = *(IUnknown**)ppvResource;
        ID3D12Resource *res = nullptr; 
        unk->QueryInterface(__uuidof(ID3D12Resource), (void**)&res);
        if(res)
            __debugbreak();

        return ret;
    }

    virtual HRESULT STDMETHODCALLTYPE CreateReservedResource(
        _In_  const D3D12_RESOURCE_DESC* pDesc,
        D3D12_RESOURCE_STATES InitialState,
        _In_opt_  const D3D12_CLEAR_VALUE* pOptimizedClearValue,
        REFIID riid,
        _COM_Outptr_opt_  void** ppvResource)
    {
        HRESULT ret = redirect->CreateReservedResource(pDesc, InitialState, pOptimizedClearValue, riid, ppvResource);

        IUnknown* unk = *(IUnknown**)ppvResource;
        ID3D12Resource* res = nullptr;
        unk->QueryInterface(__uuidof(ID3D12Resource), (void**)&res);
        if (res)
            __debugbreak();

        return ret;
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
        HRESULT ret = redirect->OpenSharedHandle(NTHandle, riid, ppvObj);

        IUnknown* unk = *(IUnknown**)ppvObj;
        ID3D12Resource* res = nullptr;
        unk->QueryInterface(__uuidof(ID3D12Resource), (void**)&res);
        if (res)
            __debugbreak();

        return ret;
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
        HRESULT ret = redirect->CreateQueryHeap(pDesc, riid, ppvHeap);

        IUnknown* unk = *(IUnknown**)ppvHeap;
        ID3D12Resource* res = nullptr;
        unk->QueryInterface(__uuidof(ID3D12Resource), (void**)&res);
        if (res)
            __debugbreak();

        return ret;
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
        HRESULT ret = redirect->CreateCommandSignature(pDesc, pRootSignature, riid, ppvCommandSignature);

        IUnknown* unk = *(IUnknown**)ppvCommandSignature;
        ID3D12Resource* res = nullptr;
        unk->QueryInterface(__uuidof(ID3D12Resource), (void**)&res);
        if (res)
            __debugbreak();

        return ret;
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

