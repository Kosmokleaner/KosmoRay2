#include "Mesh.h"
#include "Helpers.h"
#include "../external/DearGPU/OBJMeshLoader.h"
#include "../external/DearGPU/TangentSpaceCalculation.h"
#include "../external/DearGPU/forsyth.h"


void Mesh::startUpload(Renderer& renderer, VFormatFull* vertices, UINT inVertexCount, IndexType* indices, UINT inIndexCount)
{
    vertexCount = inVertexCount;
    indexCount = inIndexCount;

#if MESH_UPLOAD_METHOD == 0
    renderer.UpdateBufferResource(renderer.copyCommandList, &vertexBuffer.resource, &intermediateVertexBuffer, inVertexCount, sizeof(*vertices), vertices);
    renderer.UpdateBufferResource(renderer.copyCommandList, &indexBuffer.resource, &intermediateIndexBuffer, indexCount, sizeof(IndexType), indices);
#elif MESH_UPLOAD_METHOD == 1
    AllocateUploadBuffer(renderer.device.Get(), vertices, vertexCount * sizeof(*vertices), &vertexBuffer.resource);
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
    vertexBufferView.StrideInBytes = sizeof(VFormatFull);
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

void Mesh::load(Renderer& renderer, const wchar_t* fileName)
{
    assert(fileName);

    auto device = renderer.device;

    OBJMeshLoader Loader;

    SimpleIndexedMesh Mesh;

    if (Loader.Load(fileName, Mesh, OBJMeshLoader::OLF_Default, 1.0f))
    {
//        if (AssetKey.Flags & OBJMeshLoader::OLF_Center)
//        {
//            Mesh.CenterContent();
//        }

        //		assert(Mesh.IsValid());
        vertexStride = sizeof(VFormatFull);

        SetSimpleIndexedMesh(Mesh);

        CreateRenderMesh(renderer);
    }

//    std::vector<Mesh::IndexType> indexBufferVec;
//    std::vector<VertexPosColor> vertexBufferVec;

/*    tinyobj::ObjReaderConfig reader_config;
    reader_config.mtl_search_path = "./"; // Path to material files

    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(std::string(fileName), reader_config)) {
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
*/
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
    geometryDesc.Triangles.VertexCount = static_cast<UINT>(vertexBuffer.resource->GetDesc().Width) / sizeof(VFormatFull);
    geometryDesc.Triangles.VertexBuffer.StartAddress = vertexBuffer.resource->GetGPUVirtualAddress();
    geometryDesc.Triangles.VertexBuffer.StrideInBytes = sizeof(VFormatFull);

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



void Mesh::SetSimpleIndexedMesh(const SimpleIndexedMesh& IndexedMesh)
{
    // set vertexStride before calling this method
    assert(vertexStride);

    {
        char str[256];
        sprintf_s(str, sizeof(str) / sizeof(str[0]), "   SimpleIndexedMesh: %d Triangles, %d Pos, %d Normal, %d UVs\n", IndexedMesh.GetTriangleCount(), (int)IndexedMesh.Positions.size(), (int)IndexedMesh.Normals.size(), (int)IndexedMesh.UVs.size());
        OutputDebugStringA(str);
    }

    CTangentSpaceCalculation<SimpleIndexedMesh> TSCalc;

    TSCalc.CalculateTangentSpace(IndexedMesh);

    std::map<MultiIndex<3>, uint32> IndexMap;		// to map from (Pos, Base, UV) to (Vertex)

    std::vector<SIndexedTriangle>::const_iterator it, end = IndexedMesh.Triangles.end();

    // reserve enough to avoid reallocaion
    MeshVertexData.reserve(IndexedMesh.Triangles.size() * 3);

    // reserve exact amount
    MeshIndexData.reserve(IndexedMesh.Triangles.size() * 3);

    bool AlignToNormal = !IndexedMesh.Normals.empty();

    uint32 TriangleId = 0;
    for (it = IndexedMesh.Triangles.begin(); it != end; ++it, ++TriangleId)
    {
        const SIndexedTriangle& ref = *it;

        uint32 BaseIndices[3];
        TSCalc.GetTriangleBaseIndices(TriangleId, BaseIndices);

        // 3 vertices for each triangle
        for (uint32 v = 0; v < 3; ++v)
        {
            MultiIndex<3> SrcIndex;			// [0]:Pos, [1]:Base, [2]:UV

            SrcIndex.Index[0] = ref.Vertex[v].PositionIndex;
            SrcIndex.Index[1] = BaseIndices[v];
            SrcIndex.Index[2] = ref.Vertex[v].UVIndex;

            std::map<MultiIndex<3>, uint32>::iterator it = IndexMap.find(SrcIndex);

            uint32 Index;

            if (it == IndexMap.end())
            {
                // unknown indices create a new vertex
                Index = (uint32)MeshVertexData.size();

                IndexMap.insert(std::pair<MultiIndex<3>, uint32>(SrcIndex, Index));

                float3 Pos = IndexedMesh.Positions[SrcIndex.Index[0]];

                float3 tangentU, tangentV, normal;

                TSCalc.GetBase(BaseIndices[v], (float*)&tangentU, (float*)&tangentV, (float*)&normal);

                if (AlignToNormal)
                {
                    float3 oldNormal = IndexedMesh.Normals[ref.Vertex[v].NormalIndex];

                    tangentU = normalize(tangentU - oldNormal * dot(tangentU, oldNormal));
                    tangentV = normalize(tangentV - oldNormal * dot(tangentV, oldNormal));
                    normal = oldNormal;
                }

                // realign to given normal (good if mesh has split normals that shouldn't be split)

                //				Normal = IndexedMesh.Normals[SrcIndex.Index[1]];
                //				Normal = Normal.UnsafeNormal();

//later				TColorFixedBGRA Color = TColorFixedBGRA(MakeTVector4F(Normal.X * 0.5f + 0.5f, Normal.Y * 0.5f + 0.5f, Normal.Z * 0.5f + 0.5f, 1)); 
                XMFLOAT2 UV = XMFLOAT2(0, 0);

                if (!IndexedMesh.UVs.empty())
                {
                    UV = IndexedMesh.UVs[SrcIndex.Index[2]];
                }

                MeshVertexData.push_back(VFormatFull(Pos, tangentU, tangentV, normal, UV)); // later, Color));
            }
            else
            {
                // known indices are mapped to the existing vertex
                Index = it->second;
            }

            INDEXBUFFER_TYPE MaxIndex = ~(INDEXBUFFER_TYPE)0;

            // we only support 16 bit indexbuffer
            assert(Index <= MaxIndex);

            MeshIndexData.push_back(Index);
        }
    }

    UpdateLocalMinMax();

    {
        uint32 TriangleCount = (uint32)MeshIndexData.size() / 3;
        char str[256];
        sprintf_s(str, sizeof(str) / sizeof(str[0]), "   RenderMesh: %d Triangles, %d Vertices, %.2f Vert/Tri\n", TriangleCount, (uint32)MeshVertexData.size(), (uint32)MeshVertexData.size() / (float)TriangleCount);
        OutputDebugStringA(str);
    }


    uint32 VertexCount = (uint32)MeshVertexData.size();
    uint32 IndexCount = (uint32)MeshIndexData.size();

//    if (AssetKey.Flags & OBJMeshLoader::OLF_NoVertexCacheOptimize) // hack
    if(false)
    {
        void* VertexData = &MeshVertexData[0];
        INDEXBUFFER_TYPE* IndexData = (INDEXBUFFER_TYPE*)&MeshIndexData[0];
    }
    else
    {
        // can be optimized (wasteful memory copy)

        // vertex cache optimization
        INDEXBUFFER_TYPE* IndexData = reorderForsyth(&MeshIndexData[0], IndexCount / 3, VertexCount);
        //	uint32 Test1 = ComputeCacheHits<VERTEX_CACHE_SIZE>(IndexData, inIndexCount);

        void* VertexData = (void*)new uint8[VertexCount * vertexStride];

        // sort by new index order to get better cache locality
        {
            INDEXBUFFER_TYPE* RemapBuffer = new INDEXBUFFER_TYPE[VertexCount];

            uint32 FillState = 0;

            memset(RemapBuffer, 0xff, VertexCount * sizeof(INDEXBUFFER_TYPE));

            for (uint32 i = 0; i < IndexCount; ++i)
            {
                INDEXBUFFER_TYPE& Val = IndexData[i];

                assert(Val < VertexCount);

                INDEXBUFFER_TYPE& Remap = RemapBuffer[Val];

                if (Remap == (INDEXBUFFER_TYPE)-1)
                {
                    assert(FillState < VertexCount);

                    memcpy((uint8*)VertexData + FillState * vertexStride, (uint8*)&MeshVertexData[0] + Val * vertexStride, vertexStride);

                    Val = Remap = FillState;
                    ++FillState;
                }
                else
                {
                    Val = Remap;
                }
            }

            delete[] RemapBuffer;
        }

        MeshVertexData.clear();
        MeshVertexData.shrink_to_fit();
        MeshIndexData.clear();
        MeshIndexData.shrink_to_fit();

        MeshVertexData.resize(VertexCount);
        MeshIndexData.resize(IndexCount);

        memcpy(&MeshVertexData[0], VertexData, VertexCount * vertexStride);
        memcpy(&MeshIndexData[0], IndexData, IndexCount * sizeof(INDEXBUFFER_TYPE));

        // vertex cache optimization
        delete[] IndexData; IndexData = 0;
        delete[] VertexData; VertexData = 0;

        {
            char str[256];
            sprintf_s(str, sizeof(str) / sizeof(str[0]), "   VertexCacheOptimized");
            OutputDebugStringA(str);
        }
    }

    // create BVH
    {
        std::vector<float3> VertexDataVec;

        VertexDataVec.reserve(VertexCount);

        for (uint32 i = 0; i < VertexCount; ++i)
        {
            float3 Pos = *(float3*)((uint8*)&MeshVertexData[0] + i * vertexStride);

            VertexDataVec.push_back(Pos);
        }

        std::vector<UINT> IndexDataVec;

        IndexDataVec.reserve(IndexCount);

        for (uint32 i = 0; i < IndexCount; ++i)
        {
            IndexDataVec.push_back(MeshIndexData[i]);
        }

        uint32 TriangleCount = (uint32)MeshIndexData.size() / 3;

        BVHTree.Construct(TriangleCount, &VertexDataVec[0], &IndexDataVec[0]);

        {
            uint32 TriangleCount = (uint32)MeshIndexData.size() / 3;
            char str[256];
            //			sprintf_s(str, sizeof(str) / sizeof(str[0]), "   BVHTree: %d HalfNodes, %d Leafs, %.2f\n", BVHTree.HalfNodes.size(), BVHTree.LeafData.size(), BVHTree.ComputeAvgLeafSize());
            sprintf_s(str, sizeof(str) / sizeof(str[0]), "   BVHTree: %d HalfNodes, %d Leafs\n", (uint32)BVHTree.HalfNodes.size(), (uint32)BVHTree.LeafData.size());
            OutputDebugStringA(str);
        }
    }
}

void Mesh::UpdateLocalMinMax()
{
    LocalMin = float3(FLT_MAX, FLT_MAX, FLT_MAX);
    LocalMax = float3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

    auto end = MeshVertexData.end();
    for (auto it = MeshVertexData.begin(); it != end; ++it)
    {
        const VFormatFull& v = *it;
        float3 Pos = v.Pos;

        LocalMin = Min(LocalMin, Pos);
        LocalMax = Max(LocalMax, Pos);
    }
}

void Mesh::CreateRenderMesh(Renderer& renderer)
{
    auto device = renderer.device;

    AllocateUploadBuffer(device.Get(), MeshIndexData.data(), MeshIndexData.size() * sizeof(MeshIndexData[0]), &indexBuffer.resource);
    AllocateUploadBuffer(device.Get(), MeshVertexData.data(), MeshVertexData.size() * sizeof(MeshVertexData[0]), &vertexBuffer.resource);

    // Vertex buffer is passed to the shader along with index buffer as a descriptor range.
    UINT descriptorIndexIB = renderer.CreateBufferSRV(&indexBuffer, (UINT)MeshIndexData.size(), 2);
    UINT descriptorIndexVB = renderer.CreateBufferSRV(&vertexBuffer, (UINT)MeshVertexData.size(), sizeof(MeshVertexData[0]));
    ThrowIfFalse(descriptorIndexVB == descriptorIndexIB + 1, L"Vertex Buffer descriptor index must follow that of Index Buffer descriptor index");

}