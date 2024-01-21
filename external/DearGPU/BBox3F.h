#pragma once

#include <vector>
#include <float.h>			// FLT_MAX


class FBBox3F
{
public:
	float3 MinPos;
	float3 MaxPos;

	// default constructor
	FBBox3F() :MinPos(FLT_MAX, FLT_MAX, FLT_MAX), MaxPos(-FLT_MAX, -FLT_MAX, -FLT_MAX)
	{
	}

	// constructor
	FBBox3F(const float3& rhs) :MinPos(rhs), MaxPos(rhs)
	{
	}

	// constructor
	FBBox3F(const float3& Min, const float3& Max) :MinPos(Min), MaxPos(Max)
	{
	}

	// copy constructor (no need as XMFLOAT3 should have copy constructors and the default copy constructor will work)
//	FBBox3F(const FBBox3F& rhs) :MinPos(rhs.MinPos), MaxPos(rhs.MaxPos)
//	{
//	}

	FBBox3F& operator=(const FBBox3F& rhs)
	{
		MinPos = rhs.MinPos;
		MaxPos = rhs.MaxPos;

		return *this;
	}

	void Union(const FBBox3F &rhs)
	{
		MinPos = Min(MinPos, rhs.MinPos);
		MaxPos = Max(MaxPos, rhs.MaxPos);
	}

	void Union(const float3 &rhs)
	{
		MinPos = Min(MinPos, rhs);
		MaxPos = Max(MaxPos, rhs);
	}

	float3 ComputeCenter() const
	{
		return (MinPos + MaxPos) * 0.5f;
	}

	float3 ComputeExtent() const
	{
		return MaxPos - MinPos;
	}

	FLOAT ComputeArea() const
	{
		float3 Extent = ComputeExtent();

		return 2 * (Extent.x * Extent.y + Extent.x * Extent.z + Extent.y * Extent.z);
	}
};


// serialize
template <class CGeneralArchive>
inline void operator &(CGeneralArchive& ar, FBBox3F& In)
{
	ar & In.MinPos;
	ar & In.MaxPos;
}

// serialize
template<class CGeneralArchive>
inline void operator &(CGeneralArchive& ar, float3& g)
{
	ar & g.x;
	ar & g.y;
	ar & g.z;
}

// two FBVHHalfNode form one node
class FBVHHalfNode
{
public:
	// >0: index into HalfNodes[0,1,3,5,7,9,11,...], <=0: index into LeafData[0,1,2,3,4,5,6,...]
	int32 ChildIndex;
	// bounding box
	FBBox3F ChildBBox;
	// useful to limit the stack size during traversal
	uint32 SubtreeHeight;
};

// serialize
template <class CGeneralArchive>
inline void operator &(CGeneralArchive& ar, FBVHHalfNode& In)
{
	ar & In.ChildIndex;
	ar & In.ChildBBox;
	ar & In.SubtreeHeight;
}

// only needed during construction
class FTriangleInputLeaf
{
public:
	FTriangleInputLeaf(UINT InTriangleId, FTriangleInputLeaf* InNextLeaf = 0) : TriangleId(InTriangleId), NextLeaf(InNextLeaf)
	{
	}

	UINT TriangleId;
	// 0 is used as end terminator
	FTriangleInputLeaf* NextLeaf;
	UINT SubtreeHeight = 0;

	// count linked list length
	uint32 Count() const
	{
		uint32 Ret = 0;
		const FTriangleInputLeaf* p = this;

		while (p)
		{
			++Ret;
			p = p->NextLeaf;
		}

		return Ret;
	}
};


inline FBBox3F ComputeTriangleMinMax(UINT TriangleId, float3* VertexData, UINT* IndexData)
{
	UINT Index = TriangleId * 3;
	UINT Indices[3] = { IndexData[Index], IndexData[Index + 1], IndexData[Index + 2] };
	float3 Vertices[3] = { VertexData[Indices[0]], VertexData[Indices[1]], VertexData[Indices[2]] };

	FBBox3F ret(Vertices[0]);

	ret.Union(Vertices[1]);
	ret.Union(Vertices[2]);

	return ret;
}
