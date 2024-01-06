#include "Mock12.h"


HRESULT Mock12Device2::CreatePipelineLibrary(
_In_reads_(BlobLength)  const void* pLibraryBlob,
SIZE_T BlobLength,
REFIID riid,
_COM_Outptr_  void** ppPipelineLibrary)
{
    return E_NOTIMPL;
}

HRESULT Mock12Device2::SetEventOnMultipleFenceCompletion(
_In_reads_(NumFences)  ID3D12Fence* const* ppFences,
_In_reads_(NumFences)  const UINT64* pFenceValues,
UINT NumFences,
D3D12_MULTIPLE_FENCE_WAIT_FLAGS Flags,
HANDLE hEvent)
{
    return E_NOTIMPL;
}

HRESULT Mock12Device2::SetResidencyPriority(
UINT NumObjects,
_In_reads_(NumObjects)  ID3D12Pageable* const* ppObjects,
_In_reads_(NumObjects)  const D3D12_RESIDENCY_PRIORITY* pPriorities)
{
    return E_NOTIMPL;
}

HRESULT Mock12Device2::CreatePipelineState(
const D3D12_PIPELINE_STATE_STREAM_DESC* pDesc,
REFIID riid,
_COM_Outptr_  void** ppPipelineState)
{
    return E_NOTIMPL;
}
