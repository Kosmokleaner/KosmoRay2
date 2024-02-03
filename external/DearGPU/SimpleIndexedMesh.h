#pragma once
#include <vector>
#include "OBJMeshLoader.h"	// IIndexedMeshSink
#include "FBVHTree.h" // FBBox3F

// simple implementation, not meant to be efficient
// no materials on purpose
class SimpleIndexedMesh :public IIndexedMeshSink
{
public:
	SimpleIndexedMesh()
		: TriangleCount(0)
	{
	}

	bool IsValid() const
	{
		auto it = Triangles.begin(), end = Triangles.end();

		for(;it != end; ++it)
		{
			const SIndexedTriangle &ref = *it;

			for(uint32 i = 0; i < 3; ++i)
			{
				const SIndexedVertex Vertex = ref.Vertex[i];

				if(Vertex.PositionIndex >= Positions.size())
				{
					assert(0);
					return false;
				}
				if(Normals.empty())
				{
					if(Vertex.NormalIndex >= Normals.size())
					{
						assert(0);
						return false;
					}
				}
				if(!UVs.empty())
				{
					if(Vertex.UVIndex >= UVs.size())
					{
						assert(0);
						return false;
					}
				}
			}
		}

		return true;
	}

	uint32 GetPrimitiveStartIndex(uint32 MaterialId) const
	{
		return PrimitiveRemapping[MaterialId].StartIndex;
	}

	uint32 GetPrimitiveCount(uint32 MaterialId) const
	{
		return PrimitiveRemapping[MaterialId].Count;
	}

	void CenterContent()
	{
		FBBox3F AABB;

		for(auto it = Positions.begin(), end = Positions.end(); it !=end; ++it)
		{
            glm::vec3 &Pos = *it;

			AABB.Union(Pos);
		}

        glm::vec3 OldCenter = AABB.ComputeCenter();

		// bottom at 0,0,0
		OldCenter.y = AABB.MinPos.y;

		for(auto it = Positions.begin(), end = Positions.end(); it !=end; ++it)
		{
            glm::vec3&Pos = *it;

			Pos.x -= OldCenter.x;
			Pos.y -= OldCenter.y;
			Pos.z -= OldCenter.z;
		}
	}

	// ITriangleInputProxy -------------------------------------------------

	unsigned int GetTriangleCount() const
	{
		return (unsigned int)Triangles.size();
	}

	void GetTriangleIndices( const unsigned int indwTriNo, unsigned int outdwPos[3], unsigned int outdwNorm[3], unsigned int outdwUV[3] ) const
	{
		assert(indwTriNo < Triangles.size());
		const SIndexedTriangle &ref = Triangles[indwTriNo];

		for(uint32 i = 0; i < 3; ++i)
		{
			outdwPos[i] = ref.Vertex[i].PositionIndex;
			outdwNorm[i] = ref.Vertex[i].NormalIndex;
			outdwUV[i] = ref.Vertex[i].UVIndex;
		}
	}

	void GetPos( const unsigned int indwPos, float outfPos[3] ) const
	{
		assert(indwPos < Positions.size());
		const glm::vec3&ref = Positions[indwPos];

		outfPos[0] = ref.x;
		outfPos[1] = ref.y;
		outfPos[2] = ref.z;
	}

	void GetUV( const unsigned int InIndex, float outfUV[2] ) const
	{
		if(UVs.empty())
		{
			outfUV[0] = 0;
			outfUV[1] = 0;
			return;
		}

		assert(InIndex < UVs.size());
		const glm::vec2&ref = UVs[InIndex];

		outfUV[0] = ref.x;
		outfUV[1] = ref.y;
	}

	// interface ITriangleMeshSink -----------------------------------------

	virtual void PreparePoly(uint32 VertexCount, uint32 MaterialId)
	{
		assert(VertexCount >= 3);

		uint32 Triangles = VertexCount - 2; 

		TriangleCount += Triangles;

		GetMaterialRemapping(MaterialId).Count += Triangles;
	}

	virtual bool OnPrepare(UINT inPositions, UINT inNormals, UINT inUVs, UINT inMaterials)
	{
		// call PreparePoly() before
		assert(TriangleCount);
		assert(inMaterials);

		Triangles.resize(TriangleCount);
		Positions.resize(inPositions);
		UVs.resize(inUVs);
		Normals.resize(inNormals);
		PrimitiveRemapping.resize(inMaterials);

		BuildSummedAreaTable();
		return true;
	}

	virtual void OnFailed(const char *ErrorString)
	{
	}

	virtual void SetPos(UINT Index, glm::vec3 Value)
	{
		assert(Index < Positions.size());
		Positions[Index] = Value;
	}

	virtual void SetNormal(UINT Index, glm::vec3 Value)
	{
		assert(Index < Normals.size());
		Normals[Index] = Value;
	}

	virtual void SetUV(UINT Index, glm::vec2 Value)
	{
		assert(Index < UVs.size());
		UVs[Index] = Value;
	}

	virtual void AddPoly(const SIndexedVertex* Vertices, uint32 VertexCount, uint32 MaterialId)
	{
		assert(VertexCount >= 3);

		SPrimitive &Remapping = PrimitiveRemapping[MaterialId];

		for(uint32 i = 1; i < VertexCount - 1; ++i)
		{
			SIndexedTriangle& ref = Triangles[Remapping.StartIndex + Remapping.Count++];

			ref.MaterialId = MaterialId;

			ref.Vertex[0] = Vertices[0];
			ref.Vertex[1] = Vertices[i];
			ref.Vertex[2] = Vertices[i + 1];
		}
	}

	virtual void SetMaterial(UINT Index, const COBJMeshMaterial &Value)
	{
	}

	// ----------------------------------------------------------------------

	uint32								TriangleCount;
	std::vector<SIndexedTriangle>		Triangles;
	std::vector<glm::vec3>				Positions;
	std::vector<glm::vec3>				Normals;
	std::vector<glm::vec2>				UVs;

private: // -----------------------------------------------------------------

	struct SPrimitive
	{
		SPrimitive()
			: Count(0)
			, StartIndex(-1)
		{
		}

		// in primitives
		uint32 Count;
		// in primitives, -1 if not set yet
		uint32 StartIndex;
	};

	// to compacten the materials (all triangles of one material type are in a block) [MaterialId] = StartIndex, is modified during processing
	std::vector<SPrimitive>			PrimitiveRemapping;

	SPrimitive& GetMaterialRemapping(uint32 MaterialId)
	{
		if(MaterialId >= PrimitiveRemapping.size())
		{
			PrimitiveRemapping.resize(MaterialId + 1);
		}

		return PrimitiveRemapping[MaterialId];
	}

	void BuildSummedAreaTable()
	{
		std::vector<SPrimitive>::iterator it, end = PrimitiveRemapping.end();

		uint32 Sum = 0;

		for(it = PrimitiveRemapping.begin(); it != end; ++it)
		{
			SPrimitive& ref = *it;

			ref.StartIndex = Sum;
			Sum += ref.Count;
			ref.Count = 0;
		}
	}
};
