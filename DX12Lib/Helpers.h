/*
 *  Copyright(c) 2017 Jeremiah van Oosten
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files(the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions :
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *  IN THE SOFTWARE.
 */

/**
 *  @file Helpers.h
 *  @date August 28, 2017
 *  @author Jeremiah van Oosten
 *
 *  @brief Helper functions.
 */

#pragma once

#include <unordered_map>
#include <sstream>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h> // For HRESULT

#include <d3dx12.h>

// From DXSampleHelper.h 
// Source: https://github.com/Microsoft/DirectX-Graphics-Samples
// @param msg e.g. "one line\n"
inline void ThrowIfFailed(HRESULT hr, const wchar_t* msg = 0)
{
    if (FAILED(hr))
    {
        if(msg)
            OutputDebugString(msg);
        throw std::exception();
    }
}

// Pretty-print a state object tree.
void PrintStateObjectDesc(const D3D12_STATE_OBJECT_DESC* desc);


inline UINT Align(UINT size, UINT alignment)
{
    return (size + (alignment - 1)) & ~(alignment - 1);
}

inline void ThrowIfFalse(bool value)
{
    ThrowIfFailed(value ? S_OK : E_FAIL);
}


#define SizeOfInUint32(obj) ((sizeof(obj) - 1) / sizeof(UINT32) + 1)

#define STR1(x) #x
#define STR(x) STR1(x)
#define WSTR1(x) L##x
#define WSTR(x) WSTR1(x)
#define NAME_D3D12_OBJECT(x) x->SetName( WSTR(__FILE__ "(" STR(__LINE__) "): " L#x) )


void AllocateUAVBuffer(ID3D12Device* pDevice, ID3D12GraphicsCommandList2* commandList, UINT64 bufferSize, ID3D12Resource** ppResource, D3D12_RESOURCE_STATES initialResourceState = D3D12_RESOURCE_STATE_COMMON, const wchar_t* resourceName = nullptr);

void AllocateUploadBuffer(ID3D12Device* pDevice, void* pData, UINT64 datasize, ID3D12Resource** ppResource, const wchar_t* resourceName = nullptr);


class GpuUploadBuffer
{
public:
    ComPtr<ID3D12Resource> GetResource() { return m_resource; }

protected:
    ComPtr<ID3D12Resource> m_resource;

    GpuUploadBuffer() {}
    ~GpuUploadBuffer()
    {
        if (m_resource.Get())
        {
            m_resource->Unmap(0, nullptr);
        }
    }

    void Allocate(ID3D12Device* device, UINT bufferSize, LPCWSTR resourceName = nullptr)
    {
        auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

        auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
        ThrowIfFailed(device->CreateCommittedResource(
            &uploadHeapProperties,
            D3D12_HEAP_FLAG_NONE,
            &bufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_resource)));
        m_resource->SetName(resourceName);
    }

    uint8_t* MapCpuWriteOnly()
    {
        uint8_t* mappedData;
        // We don't unmap this until the app closes. Keeping buffer mapped for the lifetime of the resource is okay.
        CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
        ThrowIfFailed(m_resource->Map(0, &readRange, reinterpret_cast<void**>(&mappedData)));
        return mappedData;
    }
};

// copied from DxSampleHelper.h, Microsoft, MIT license
//
// Helper class to create and update a constant buffer with proper constant buffer alignments.
// Usage: 
//    ConstantBuffer<...> cb;
//    cb.Create(...);
//    cb.staging.var = ... ; | cb->var = ... ; 
//    cb.CopyStagingToGPU(...);
//    Set...View(..., cb.GputVirtualAddress());
template <class T>
class ConstantBuffer : public GpuUploadBuffer
{
    uint8_t* m_mappedConstantData;
    UINT m_alignedInstanceSize;
    UINT m_numInstances;

public:
    ConstantBuffer() : m_alignedInstanceSize(0), m_numInstances(0), m_mappedConstantData(nullptr) {}

    void Create(ID3D12Device* device, UINT numInstances = 1, LPCWSTR resourceName = nullptr)
    {
        m_numInstances = numInstances;
        m_alignedInstanceSize = Align(sizeof(T), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
        UINT bufferSize = numInstances * m_alignedInstanceSize;
        Allocate(device, bufferSize, resourceName);
        m_mappedConstantData = MapCpuWriteOnly();
    }

    void CopyStagingToGpu(UINT instanceIndex = 0)
    {
        memcpy(m_mappedConstantData + instanceIndex * m_alignedInstanceSize, &staging, sizeof(T));
    }

    // Accessors
    T staging;
    T* operator->() { return &staging; }
    UINT NumInstances() { return m_numInstances; }
    D3D12_GPU_VIRTUAL_ADDRESS GpuVirtualAddress(UINT instanceIndex = 0)
    {
        return m_resource->GetGPUVirtualAddress() + instanceIndex * m_alignedInstanceSize;
    }
};

// Shader record = {{Shader ID}, {RootArguments}}
class ShaderRecord
{
public:
    ShaderRecord(void* pShaderIdentifier, UINT shaderIdentifierSize) :
        shaderIdentifier(pShaderIdentifier, shaderIdentifierSize)
    {
    }

    ShaderRecord(void* pShaderIdentifier, UINT shaderIdentifierSize, void* pLocalRootArguments, UINT localRootArgumentsSize) :
        shaderIdentifier(pShaderIdentifier, shaderIdentifierSize),
        localRootArguments(pLocalRootArguments, localRootArgumentsSize)
    {
    }

    void CopyTo(void* dest) const
    {
        uint8_t* byteDest = static_cast<uint8_t*>(dest);
        memcpy(byteDest, shaderIdentifier.ptr, shaderIdentifier.size);
        if (localRootArguments.ptr)
        {
            memcpy(byteDest + shaderIdentifier.size, localRootArguments.ptr, localRootArguments.size);
        }
    }

    struct PointerWithSize {
        void* ptr;
        UINT size;

        PointerWithSize() : ptr(nullptr), size(0) {}
        PointerWithSize(void* _ptr, UINT _size) : ptr(_ptr), size(_size) {};
    };
    PointerWithSize shaderIdentifier;
    PointerWithSize localRootArguments;
};


// Shader table = {{ ShaderRecord 1}, {ShaderRecord 2}, ...}
class ShaderTable : public GpuUploadBuffer
{
    uint8_t* m_mappedShaderRecords;
    UINT m_shaderRecordSize;

    // Debug support
    std::wstring m_name;
    std::vector<ShaderRecord> m_shaderRecords;

    ShaderTable() {}
public:
    ShaderTable(ID3D12Device* device, UINT numShaderRecords, UINT shaderRecordSize, LPCWSTR resourceName = nullptr)
        : m_name(resourceName)
    {
        m_shaderRecordSize = Align(shaderRecordSize, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);
        m_shaderRecords.reserve(numShaderRecords);
        UINT bufferSize = numShaderRecords * m_shaderRecordSize;
        Allocate(device, bufferSize, resourceName);
        m_mappedShaderRecords = MapCpuWriteOnly();
    }

    void push_back(const ShaderRecord& shaderRecord)
    {
        ThrowIfFalse(m_shaderRecords.size() < m_shaderRecords.capacity());
        m_shaderRecords.push_back(shaderRecord);
        shaderRecord.CopyTo(m_mappedShaderRecords);
        m_mappedShaderRecords += m_shaderRecordSize;
    }

    UINT GetShaderRecordSize() { return m_shaderRecordSize; }

    // Pretty-print the shader records.
    void DebugPrint(std::unordered_map<void*, std::wstring> shaderIdToStringMap)
    {
        std::wstringstream wstr;
        wstr << L"|--------------------------------------------------------------------\n";
        wstr << L"|Shader table - " << m_name.c_str() << L": "
            << m_shaderRecordSize << L" | "
            << m_shaderRecords.size() * m_shaderRecordSize << L" bytes\n";

        for (UINT i = 0; i < m_shaderRecords.size(); i++)
        {
            wstr << L"| [" << i << L"]: ";
            wstr << shaderIdToStringMap[m_shaderRecords[i].shaderIdentifier.ptr] << L", ";
            wstr << m_shaderRecords[i].shaderIdentifier.size << L" + " << m_shaderRecords[i].localRootArguments.size << L" bytes \n";
        }
        wstr << L"|--------------------------------------------------------------------\n";
        wstr << L"\n";
        OutputDebugStringW(wstr.str().c_str());
    }
};
