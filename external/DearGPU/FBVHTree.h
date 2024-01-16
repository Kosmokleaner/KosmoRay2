#pragma once

#include <vector>
#include "BBox3F.h"

class FBVHTree
{
public:

	// VertexData[VertexId]
	// IndexData[TriangelId*3 + 0/1/2]
	void Construct(UINT TriangleCount, float3* VertexData, UINT* IndexData);

	// @return Index
	UINT AddHalfNode();

	FBVHHalfNode& GetHalfNode(UINT Index);

	// @return index in LeafData[]
	UINT AddLeafData(FTriangleInputLeaf* Content);

	template<class CGeneralArchive>
	void serialize(CGeneralArchive & ar)
	{
		ar & HalfNodes;
		ar & LeafData;
	}

	void Free()
	{
		HalfNodes.clear();
		LeafData.clear();
	}

//private:

	// nodes and leafs
	std::vector<FBVHHalfNode>	HalfNodes;
	// Each Leaf indexes into a section, a section enumerates 1 or more triangle indices and ends the last one having the top bit 1
	std::vector<uint32>			LeafData;

private:

	// @return depth of the subtree
	uint32 UpdateSubtreeHeights(uint32 Index = 0);
};
