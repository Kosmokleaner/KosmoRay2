#pragma once
#include "Renderer.h"
#include "external/DearGPU/SimpleIndexedMesh.h"

#define REFLECTION_HELPER_MODE 1 // 2: implement reflection functions in one .cpp file
#include "ReflectionHelper.h" // set defines for next line, undefine REFLECTION_HELPER_MODE
#include "DataDefinitions.h" // your types go here
#include "ReflectionHelper.h" // undefine all

// 0:UpdateBufferResource / 1:AllocateUploadBuffer(less code complexity, no commandlist need)
#define MESH_UPLOAD_METHOD 1



class CInputElementDesc
{
	// copy constructor is private to prevent bad access pattern
	CInputElementDesc(const CInputElementDesc& rhs)
	{
	}
	// assignment operator is private to prevent bad access pattern
	const CInputElementDesc& operator=(const CInputElementDesc& rhs) const
	{
		return *this;
	}

public:
	CInputElementDesc(D3D12_INPUT_ELEMENT_DESC* InPointer, uint32 InCount)
		:Pointer(InPointer), Count(InCount)
	{
	}

	D3D12_INPUT_ELEMENT_DESC* Pointer = 0;
	uint32 Count = 0;
};

// vertex format
// used for: mesh rendering
struct VFormatFull
{
	VFormatFull()
	{
	}

	VFormatFull(glm::vec3 InPos, glm::vec3 InTangentU, glm::vec3 InTangentV, glm::vec3 InTangentN, glm::vec2 InUV)
		: Pos(InPos)
		, TangentN(InTangentN)
		, UV(InUV)
	{
	}

    glm::vec3 Pos;
    glm::vec3 TangentN;
    glm::vec2 UV;

	// todo - hide DX at that level
	static const CInputElementDesc& GetDesc()
	{
		static D3D12_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			//			{ "TEXCOORD",	0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			//			{ "TEXCOORD",	1, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD",	2, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD",	3, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			//later	{ "COLOR",		0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};

		static CInputElementDesc ret(layout, (uint32)(sizeof(layout) / sizeof(layout[0])));

		return ret;
	}
};


template <int Dimension>
struct MultiIndex
{
	uint32 Index[Dimension];	// 0:Pos, 1:Normal 2:UV

	bool operator<(const MultiIndex& rhs) const
	{
		for (uint32 i = 0; i < Dimension; ++i)
		{
			if (Index[i] < rhs.Index[i])
			{
				return true;
			}
			else if (Index[i] > rhs.Index[i])
			{
				return false;
			}
		}

		return false;
	}
};


// a triangle mesh with index and vertex buffer
class Mesh
{
public:
//    typedef WORD IndexType;	// 16 bit index buffer
	typedef uint32 IndexType;	// 32 bit index buffer
	typedef INDEXBUFFER_TYPE IndexType;

    // @param filename e.g. L"../../data/monkey.obj", must not be 0
	// @return success
    bool load(Renderer& renderer, const wchar_t* fileName);
	//
    void startUpload(Renderer& renderer, VFormatFull* vertices, UINT inVertexCount, IndexType* indices, UINT inIndexCount);

    void SetSimpleIndexedMesh(const SimpleIndexedMesh& IndexedMesh);
        
    void end();

	// @param index 0:IB, 1:VB, 2:Materials
	// @return base descriptor index
	UINT CreateSRVs(Renderer& renderer, uint32 index);

    // like FreeData / ReleaseDeviceDependentResources
    void Reset();

    UINT vertexCount = 0;
    UINT indexCount = 0;

    D3DBuffer vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView;

    D3DBuffer indexBuffer;
    D3D12_INDEX_BUFFER_VIEW indexBufferView;

	D3DBuffer materialBuffer;
	D3D12_INDEX_BUFFER_VIEW materialBufferView;

    // for ray tracing
    ComPtr<ID3D12Resource> bottomLevelAccelerationStructure;


    // needs to be 32bit! can be sizeof(SPosOnlyVertex) or sizeof(VFormatFull)
    uint32 vertexStride = 0;

    // call UpdateLocalMinMax() to recompute
    glm::vec3 LocalMin, LocalMax;

    // -1 if not set, index into CBVHTreeData.dataBVHTree0[]
    uint32 BVHRootNodeId;

    // redundant but nice for debugging
    FBVHTree BVHTree;

	// vertex data
    std::vector<VFormatFull> MeshVertexData;
	// indices into MeshVertexData, 0 based
    std::vector<INDEXBUFFER_TYPE> MeshIndexData;
	// [subSetId] = materialAttributes
	std::vector<MaterialAttributes> materialAttributes;

    // build bottomLevelAccelerationStructure
    void BuildAccelerationStructures(Renderer& renderer);

	void CreateRenderMesh(Renderer& renderer);

private:
    void init();
	void UpdateLocalMinMax();

#if MESH_UPLOAD_METHOD == 0
    ComPtr<ID3D12Resource> intermediateVertexBuffer;
    ComPtr<ID3D12Resource> intermediateIndexBuffer;
#endif
};

