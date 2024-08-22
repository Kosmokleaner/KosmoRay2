#include "Splats.h"
#include "DX12Lib/Helpers.h"

void Splats::CreateRenderMesh(Renderer& renderer)
{
	auto device = renderer.device;

	AllocateUploadBuffer(device.Get(), splatData.data(), splatData.size() * sizeof(splatData[0]), &splatBuffer.resource, L"SplatBuffer");

	NAME_D3D12_OBJECT(splatBuffer.resource);

	init();
}

void Splats::init()
{
    assert(!splatData.empty());

	splatBufferView.BufferLocation = splatBuffer.resource->GetGPUVirtualAddress();
	splatBufferView.StrideInBytes = sizeof(SplatData);
	splatBufferView.SizeInBytes = (UINT)splatData.size() * splatBufferView.StrideInBytes;
}

void Splats::Reset()
{
	splatBuffer.resource.Reset();
	bottomLevelAccelerationStructure.Reset();
}

void Splats::BuildAccelerationStructures(Renderer& renderer)
{
	auto device = renderer.device;

    std::vector<D3D12_RAYTRACING_AABB> aabs;

    aabs.reserve(splatData.size());
    for (const auto& el: splatData)
    {
        D3D12_RAYTRACING_AABB aabb;

        aabb.MinX = el.Pos.x - el.radius;
		aabb.MinY = el.Pos.y - el.radius;
		aabb.MinZ = el.Pos.z - el.radius;
		aabb.MaxX = el.Pos.x + el.radius;
		aabb.MaxY = el.Pos.y + el.radius;
		aabb.MaxZ = el.Pos.z + el.radius;

        aabs.push_back(aabb);
    }
	AllocateUploadBuffer(device.Get(), aabs.data(), aabs.size() * sizeof(aabs[0]), &aabbBuffer.resource, L"SplatAABBs");


    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;

    auto commandQueue = renderer.directCommandQueue;
    assert(commandQueue);
    auto commandList = commandQueue->GetCommandList();
    
	// PERFORMANCE TIP: mark geometry as opaque whenever applicable as it can enable important ray processing optimizations.
    D3D12_RAYTRACING_GEOMETRY_FLAGS geometryFlags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;// D3D12_RAYTRACING_GEOMETRY_FLAG_NONE;
//	geometryFlags |= D3D12_RAYTRACING_GEOMETRY_FLAG_NO_DUPLICATE_ANYHIT_INVOCATION;

    D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc = {};
	geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_PROCEDURAL_PRIMITIVE_AABBS;
	geometryDesc.AABBs.AABBCount = (UINT64)splatData.size();
	geometryDesc.AABBs.AABBs.StrideInBytes = sizeof(D3D12_RAYTRACING_AABB);
    geometryDesc.AABBs.AABBs.StartAddress = aabbBuffer.resource->GetGPUVirtualAddress();
	geometryDesc.Flags = geometryFlags;

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO bottomLevelPrebuildInfo = {};
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS bottomLevelInputs = {};
    bottomLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
    bottomLevelInputs.Flags = buildFlags;
    bottomLevelInputs.NumDescs = 1;
    bottomLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
    bottomLevelInputs.pGeometryDescs = &geometryDesc;
    renderer.dxrDevice->GetRaytracingAccelerationStructurePrebuildInfo(&bottomLevelInputs, &bottomLevelPrebuildInfo);
    ThrowIfFalse(bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes > 0);

    ComPtr<ID3D12Resource> scratchResource;
    AllocateUAVBuffer(device.Get(), commandList.Get(), bottomLevelPrebuildInfo.ScratchDataSizeInBytes, &scratchResource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, L"ScratchResource");

    {
        D3D12_RESOURCE_STATES initialResourceState = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;

        AllocateUAVBuffer(device.Get(), commandList.Get(), bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes, &bottomLevelAccelerationStructure, initialResourceState, L"BLAS");
    }

    // Bottom Level Acceleration Structure desc
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC bottomLevelBuildDesc = {};
    {
        bottomLevelBuildDesc.Inputs = bottomLevelInputs;
        bottomLevelBuildDesc.ScratchAccelerationStructureData = scratchResource->GetGPUVirtualAddress();
        bottomLevelBuildDesc.DestAccelerationStructureData = bottomLevelAccelerationStructure->GetGPUVirtualAddress();
    }

    // Build acceleration structure.
    // hack
    ComPtr<ID3D12GraphicsCommandList4> m_dxrCommandList;
    ThrowIfFailed(commandList->QueryInterface(IID_PPV_ARGS(&m_dxrCommandList)), L"Couldn't get DirectX Raytracing interface for the command list.\n");
    // BuildAccelerationStructure
    {
        auto* raytracingCommandList = m_dxrCommandList.Get();
        raytracingCommandList->BuildRaytracingAccelerationStructure(&bottomLevelBuildDesc, 0, nullptr);
        CD3DX12_RESOURCE_BARRIER a = CD3DX12_RESOURCE_BARRIER::UAV(bottomLevelAccelerationStructure.Get());
        commandList->ResourceBarrier(1, &a);
    }

    // Kick off acceleration structure construction.
    auto fenceValue = commandQueue->ExecuteCommandList(commandList);
    commandQueue->WaitForFenceValue(fenceValue);
}

