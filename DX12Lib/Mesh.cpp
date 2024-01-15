#include "Mesh.h"
#include "Helpers.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "../external/tiny_obj_loader.h"


void Mesh::startUpload(Renderer& renderer, VertexPosColor* vertices, UINT inVertexCount, IndexType* indices, UINT inIndexCount)
{
    vertexCount = inVertexCount;
    indexCount = inIndexCount;

#if MESH_UPLOAD_METHOD == 0
    renderer.UpdateBufferResource(renderer.copyCommandList, &vertexBuffer.resource, &intermediateVertexBuffer, inVertexCount, sizeof(VertexPosColor), vertices);
    renderer.UpdateBufferResource(renderer.copyCommandList, &indexBuffer.resource, &intermediateIndexBuffer, indexCount, sizeof(IndexType), indices);
#elif MESH_UPLOAD_METHOD == 1
    AllocateUploadBuffer(renderer.device.Get(), vertices, vertexCount * sizeof(VertexPosColor), &vertexBuffer.resource);
    AllocateUploadBuffer(renderer.device.Get(), indices, indexCount * sizeof(IndexType), &indexBuffer.resource);
#endif

    NAME_D3D12_OBJECT(vertexBuffer.resource);
    NAME_D3D12_OBJECT(indexBuffer.resource);

   // Vertex buffer is passed to the shader along with index buffer as a descriptor range.
//   UINT descriptorIndexIB = CreateBufferSRV(&mesh.indexBuffer, (UINT)indexBuffer.size(), 2);

    init();
}

void Mesh::end()
{
#if MESH_UPLOAD_METHOD == 0
    intermediateVertexBuffer.Detach();
    intermediateIndexBuffer.Detach();
#endif
}

void Mesh::init()
{
    assert(vertexCount);
    assert(indexCount);

    // vertex buffer view
    vertexBufferView.BufferLocation = vertexBuffer.resource->GetGPUVirtualAddress();
    vertexBufferView.StrideInBytes = sizeof(VertexPosColor);
    vertexBufferView.SizeInBytes = vertexCount * vertexBufferView.StrideInBytes;

    // index buffer view
    indexBufferView.BufferLocation = indexBuffer.resource->GetGPUVirtualAddress();
    indexBufferView.Format = DXGI_FORMAT_R16_UINT;
    assert(sizeof(IndexType) == 2);
    indexBufferView.SizeInBytes = indexCount * sizeof(IndexType);
}

void Mesh::Reset()
{
    indexBuffer.resource.Reset();
    vertexBuffer.resource.Reset();
    bottomLevelAccelerationStructure.Reset();
}

void Mesh::load(Renderer& renderer, const char* fileName)
{
    assert(fileName);
    auto device = renderer.device;
    std::string inputfile = "../../data/monkey.obj";        // 1 shape
//    std::string inputfile = "../../data/NewXYZ.obj";          // many shapes
//    std::string inputfile = "../../data/LShape.obj";    // no clipping errors
//    std::string inputfile = "../../data/saucer.obj";
    //    std::string inputfile = "../../data/GroundPlane.obj";
    tinyobj::ObjReaderConfig reader_config;
    reader_config.mtl_search_path = "./"; // Path to material files

    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(inputfile, reader_config)) {
        if (!reader.Error().empty()) {
            //            std::cerr << "TinyObjReader: " << reader.Error();
        }
        exit(1);
    }

    if (!reader.Warning().empty()) {
        //        std::cout << "TinyObjReader: " << reader.Warning();
    }

    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();
    auto& materials = reader.GetMaterials();

    // for now unoptimized, see https://vulkan-tutorial.com/Loading_models
    std::vector<Mesh::IndexType> indexBufferVec;
    std::vector<VertexPosColor> vertexBufferVec;

    // if reader_config.triangulate we can use the indexbuffer more or less directly

    // build indexBuffer
    {
        size_t indexCount = 0;
        for (size_t s = 0; s < shapes.size(); s++) {
            indexCount += shapes[s].mesh.indices.size();
        }
        indexBufferVec.resize(indexCount);
        indexCount = 0;
        for (size_t s = 0; s < shapes.size(); s++) {
            size_t size = shapes[s].mesh.indices.size();
            for (size_t i = 0; i < size; i++) {
                // for now no per vertex data e.g. normal, UVs
                indexBufferVec[indexCount++] = shapes[s].mesh.indices[i].vertex_index;
            }
        }
    }

    // build vertexBuffer
    {
        size_t vertexCount = attrib.vertices.size() / 3;
        vertexBufferVec.resize(vertexCount);
        for (size_t i = 0; i < vertexCount; ++i) {
            tinyobj::real_t vx = attrib.vertices[3 * i + 0];
            tinyobj::real_t vy = attrib.vertices[3 * i + 1];
            tinyobj::real_t vz = attrib.vertices[3 * i + 2];

            vertexBufferVec[i].Position.x = vx * 0.5f;
            vertexBufferVec[i].Position.y = vy * 0.5f;
            vertexBufferVec[i].Position.z = vz * 0.5f;
        }
    }
    AllocateUploadBuffer(device.Get(), indexBufferVec.data(), indexBufferVec.size() * sizeof(indexBufferVec[0]), &indexBuffer.resource);
    AllocateUploadBuffer(device.Get(), vertexBufferVec.data(), vertexBufferVec.size() * sizeof(vertexBufferVec[0]), &vertexBuffer.resource);

    // Vertex buffer is passed to the shader along with index buffer as a descriptor range.
    UINT descriptorIndexIB = renderer.CreateBufferSRV(&indexBuffer, (UINT)indexBufferVec.size(), 2);
    UINT descriptorIndexVB = renderer.CreateBufferSRV(&vertexBuffer, (UINT)vertexBufferVec.size(), sizeof(vertexBufferVec[0]));
    ThrowIfFalse(descriptorIndexVB == descriptorIndexIB + 1, L"Vertex Buffer descriptor index must follow that of Index Buffer descriptor index");
}

void Mesh::BuildAccelerationStructures(Renderer& renderer)
{
    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
    auto device = renderer.device;

    auto commandQueue = renderer.directCommandQueue;
    assert(commandQueue);
    auto commandList = commandQueue->GetCommandList();

    D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc = {};
    geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
    geometryDesc.Triangles.IndexBuffer = indexBuffer.resource->GetGPUVirtualAddress();
    geometryDesc.Triangles.IndexCount = static_cast<UINT>(indexBuffer.resource->GetDesc().Width) / sizeof(Mesh::IndexType);
    geometryDesc.Triangles.IndexFormat = DXGI_FORMAT_R16_UINT;
    geometryDesc.Triangles.Transform3x4 = 0;
    geometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
    geometryDesc.Triangles.VertexCount = static_cast<UINT>(vertexBuffer.resource->GetDesc().Width) / sizeof(VertexPosColor);
    geometryDesc.Triangles.VertexBuffer.StartAddress = vertexBuffer.resource->GetGPUVirtualAddress();
    geometryDesc.Triangles.VertexBuffer.StrideInBytes = sizeof(VertexPosColor);

    // Mark the geometry as opaque. 
    // PERFORMANCE TIP: mark geometry as opaque whenever applicable as it can enable important ray processing optimizations.
    // Note: When rays encounter opaque geometry an any hit shader will not be executed whether it is present or not.
    geometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
    //    geometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_NONE;
        // needed for clipping shader
    geometryDesc.Flags |= D3D12_RAYTRACING_GEOMETRY_FLAG_NO_DUPLICATE_ANYHIT_INVOCATION;

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
