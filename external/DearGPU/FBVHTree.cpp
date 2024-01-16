
#include "deargpu.h"

#include "FBVHTree.h"


// O(n), n is the list length
// @param InputContent can be 0
uint32 ComputeCount(FTriangleInputLeaf* InputContent)
{
	uint32 Ret = 0;

	while(InputContent)
	{
		++Ret;

		InputContent = InputContent->NextLeaf;
	}

	return Ret;
}

// @return smaller is better
FLOAT ComputeHeuristicBVH(FTriangleInputLeaf* InputContent, UINT SplitAxis, FLOAT SplitPos, float3* VertexData, UINT* IndexData)
{
	assert(SplitAxis < 3);
	assert(VertexData);
	assert(IndexData);

	FBBox3F SplitContentBBox[2];
	UINT SplitCount[2] = { 0, 0 };

	while(InputContent)
	{
		// could be optimized
		FBBox3F TriangleBBox = ComputeTriangleMinMax(InputContent->TriangleId, VertexData, IndexData);
		float3 TriangleCenter = TriangleBBox.ComputeCenter();

		UINT Side = TriangleCenter[SplitAxis] < SplitPos ? 0 : 1;

		SplitContentBBox[Side].Union(TriangleBBox);
		SplitCount[Side]++;

		InputContent = InputContent->NextLeaf;
	}

	if(!SplitCount[0] || !SplitCount[1])
	{
		return FLT_MAX;
	}

	return SplitContentBBox[0].ComputeArea() * SplitCount[0] + SplitContentBBox[1].ComputeArea() * SplitCount[1];
}

// @return Tree node index, >0: index into Nodes[], <=0: index into LeafData[]
static int RecursivelyProcessBVH(FBVHTree& Tree, FTriangleInputLeaf* InputContent, FBBox3F InputContentBBox, float3* VertexData, UINT* IndexData)
{
	// find best split position and axis
	UINT BestSplitAxis = 0;
	FLOAT BestSplitPos = 0;

	FLOAT BestHeuristic = FLT_MAX;

	// quite brute force technique, using SAH and x split planes we try to find the best one
	{
		for(UINT Axis = 0; Axis < 3; ++Axis)
		{
			// more would be better for raycasting later on but also slower during preprocess
			const UINT TestCount = BVH_TEST_SPLIT_PLANES;
		
			for(UINT Test = 0 ; Test < TestCount; ++Test)
			{
				FLOAT SplitPos = lerp(InputContentBBox.MinPos[Axis], InputContentBBox.MaxPos[Axis], (Test + 1.0f) / (TestCount + 2.0f));

				FLOAT Heuristic = ComputeHeuristicBVH(InputContent, Axis, SplitPos, VertexData, IndexData);

				if(Heuristic < BestHeuristic)
				{
					BestHeuristic = Heuristic;
					BestSplitAxis = Axis;
					BestSplitPos = SplitPos;
				}
			}
		}
	}

	// [0=left, 1=right]
	FTriangleInputLeaf* SplitContent[2] = { 0, 0 };
	// [0=left, 1=right]
	FBBox3F SplitContentBBox[2];

	// split content into two parts
	{
		FTriangleInputLeaf* Leaf = InputContent;
		while(Leaf)
		{
			FTriangleInputLeaf* NextLeaf = Leaf->NextLeaf;

			FBBox3F TriangleBBox = ComputeTriangleMinMax(Leaf->TriangleId, VertexData, IndexData);
			float3 TriangleCenter = TriangleBBox.ComputeCenter();

			UINT Side = TriangleCenter[BestSplitAxis] < BestSplitPos ? 0 : 1;

			Leaf->NextLeaf = SplitContent[Side];
			SplitContent[Side] = Leaf;
			SplitContentBBox[Side].Union(TriangleBBox);

			Leaf = NextLeaf;
		}
	}

	// only needed for a better heuristic
//	uint32 Count0 = ComputeCount(SplitContent[0]);
//	uint32 Count1 = ComputeCount(SplitContent[1]);

	if(!SplitContent[0] || !SplitContent[1])		
//	if(Count0 < 1 || Count1 < 1 || Count0 + Count1 < 3)	// seems any other heuristic to stop is slower
	{
		FTriangleInputLeaf* Content = SplitContent[0] ? SplitContent[0] : SplitContent[1];

		assert(Content);

		int LeafDataIndex = Tree.AddLeafData(Content);

		// leaf node
		return -LeafDataIndex;
//		return -1;
	}

	UINT ret = Tree.AddHalfNode();

	Tree.AddHalfNode();

	// note that we dont' store a pointer or reference to Tree.GetHalfNode(ret) as the HalfNodes might get reallocated in the recursive call
	Tree.GetHalfNode(ret).ChildBBox = SplitContentBBox[0];

	{
		int k = RecursivelyProcessBVH(Tree, SplitContent[0], SplitContentBBox[0], VertexData, IndexData);

		Tree.GetHalfNode(ret).ChildIndex = k;
	}

	Tree.GetHalfNode(ret + 1).ChildBBox = SplitContentBBox[1];

	{
		int k = RecursivelyProcessBVH(Tree, SplitContent[1], SplitContentBBox[1], VertexData, IndexData);

		Tree.GetHalfNode(ret + 1).ChildIndex = k;
	}

	return ret;
}

void FBVHTree::Construct(UINT TriangleCount, float3* VertexData, UINT* IndexData)
{
	FBBox3F ContentBBox;

	std::vector<FTriangleInputLeaf> TempLeafs;

	TempLeafs.reserve(TriangleCount);

	// this is the start of the linked list
	FTriangleInputLeaf* AllTriangles = 0;

	// connect all triangles in a linked list
	for(UINT i = 0; i < TriangleCount; ++i)
	{
		TempLeafs.push_back(FTriangleInputLeaf(i, AllTriangles));

		AllTriangles = &TempLeafs[i];

		FBBox3F TriangleBBox = ComputeTriangleMinMax(i, VertexData, IndexData);

		ContentBBox.Union(TriangleBBox);
	}

	// there is existing tree data, either clear it (to update the tree) out or don't call the Construct twice
	assert(HalfNodes.size() == 0);

	UINT RootNodeIndex = AddHalfNode();

	assert(RootNodeIndex == 0);

	GetHalfNode(RootNodeIndex).ChildBBox = ContentBBox;
	{
		int k = RecursivelyProcessBVH(*this, AllTriangles, ContentBBox, VertexData, IndexData);

		GetHalfNode(RootNodeIndex).ChildIndex = k;
	}

	UpdateSubtreeHeights();
}

uint32 FBVHTree::UpdateSubtreeHeights(uint32 Index)
{
	FBVHHalfNode& Node = HalfNodes[Index];

	uint32 Ret = 0;

	if(Node.ChildIndex > 0)
	{
		// ChildIndex is index into HalfNodes[]

		uint32 DepthL = UpdateSubtreeHeights(Node.ChildIndex);
		uint32 DepthR = UpdateSubtreeHeights(Node.ChildIndex + 1);

		Ret = Max(DepthL, DepthR) + 1;
	}

	Node.SubtreeHeight = Ret;
	return Ret;
}

UINT FBVHTree::AddHalfNode()
{
	UINT ret = (UINT)HalfNodes.size();

	HalfNodes.push_back(FBVHHalfNode());

	return ret;
}

FBVHHalfNode& FBVHTree::GetHalfNode(UINT Index)
{
	assert(Index < HalfNodes.size());
	return HalfNodes[Index];
}

UINT FBVHTree::AddLeafData(FTriangleInputLeaf* Content)
{
	UINT ret = (UINT)LeafData.size();
	
	for(;;)
	{
		auto next = Content->NextLeaf;

		uint32 data = Content->TriangleId * 3;

		if(!next)
		{
			// top bit is used as terminator
			LeafData.push_back(data | 0x80000000);
			break;
		}

		LeafData.push_back(data);

		Content = next;
	}

	return ret;
}
