#pragma once

#include "deargpu.h"
#include <string>
#include <vector>
#include <d3d11.h>
#include <DirectXMath.h>

using namespace DirectX;

// features:
// * does not reconstruct normals, doesn't intepret smoothing groups
// * ignores content it doesn't understand
// * supports relative and absolute indices

class COBJMeshMaterial
{
public:
	std::string			MaterialName;

	float3			DiffuseColor;
	float3			SpecularColor;

	std::wstring		DiffuseTexture;
	std::wstring		SpecularTexture;
	std::wstring		BumpTexture;
	
	float				SpecularPower;

	// constructor
	COBJMeshMaterial()
		: SpecularPower(64.0f)
	{
		DiffuseColor = float3(1, 1, 1);
		SpecularColor = float3(1, 1, 0);
	}
};

// absolute, 0 based indices
struct SIndexedVertex
{
	void Reset()
	{
		PositionIndex = 0;
		NormalIndex = 0;
		UVIndex = 0;
	}

	uint32			PositionIndex;
	uint32			NormalIndex;
	uint32			UVIndex;
};


// absolute, 0 based indices
struct SIndexedTriangle
{
	void Reset()
	{
		for(int i = 0; i < 3; ++i)
		{
			Vertex[i].Reset();
		}
		MaterialId = 0;
	}

	// 3 to get one for each triangle vertex
	SIndexedVertex Vertex[3];
	// on material index per triangle 
	uint32			MaterialId;
};

struct IIndexedMeshSink
{
	virtual void PreparePoly(uint32 VertexCount, uint32 MaterialId) = 0;

	virtual bool OnPrepare(uint32 Positions, uint32 Normals, uint32 UVs, uint32 Materials) = 0;

	virtual void OnFailed(const char* ErrorString) = 0;

	virtual void SetPos(uint32 Index, float3 Value) = 0;

	virtual void SetNormal(uint32 Index, float3 Value) = 0;

	virtual void SetUV(uint32 Index, XMFLOAT2 Value) = 0;

	virtual void AddPoly(const SIndexedVertex* Vertices, uint32 VertexCount, uint32 MaterialId) = 0;

	virtual void SetMaterial(uint32 Index, const COBJMeshMaterial &Value) = 0;
};



class OBJMeshLoader
{
public:

	// bitmask
	enum EOBJLoaderFlags
	{
		OLF_Default = 0,
		OLF_Reorient = 1,
		OLF_Center = 2,
		OLF_NoImportNormals = 4,
		OLF_NoVertexCacheOptimize = 8,		// 14 faster for generator_small_high.obj if using a vertex expensive VS, not noticable with cheap VS
	};

	// constructor
	OBJMeshLoader();

	// FileName must not be 0
	bool Load(const wchar_t* FileName, IIndexedMeshSink &Sink, EOBJLoaderFlags Flags, float Scale = 1.0f);

	// asserts if outside bounds
	COBJMeshMaterial &GetOBJMaterial(uint32 SubSetId);

	//
	uint32 GetOBJMaterialCount() const;

private: // -------------------------------------------------------

	//
	static void ParseWhiteSpace(uint8 * &BufferPos );

	// not case sensitive
	// will not change BufferPos if the parsing wasn't successful
	// also jumps over following whitespace
	static bool ParseName(uint8 * &BufferPos, const char *Name);

	//
	// will not change BufferPos if the parsing wasn't successful
	// also jumps over following whitespace
	static bool ParseValue(uint8 * &BufferPos, int &outValue);

	//
	// will not change BufferPos if the parsing wasn't successful
	// also jumps over following whitespace
	static bool ParseValue(uint8 * &BufferPos, float &outValue);

	//
	// will not change BufferPos if the parsing wasn't successful
	// also jumps over following whitespace
	static bool ParseValue(uint8 * &BufferPos, float3 &outValue);

	//
	// will not change BufferPos if the parsing wasn't successful
	// also jumps over following whitespace
	static bool ParseValue(uint8 * &BufferPos, XMFLOAT2 &outValue);

	// can be optimized
	static void ParseName(uint8 * & p, std::string &Out);

	//
	static void ParseFileName(uint8 * & p, std::wstring &Out);

	static bool IsNameCharacter(uint8 Value);

	//
	bool LoadMeshPass1(const wchar_t*FileName, IIndexedMeshSink &Sink, uint32 &Positions, uint32 &Normals, uint32 &UVs);

	//
	bool LoadMeshPass2(const wchar_t*FileName, IIndexedMeshSink &Sink, EOBJLoaderFlags Flags, float Scale);

	//
	bool LoadMTL(const wchar_t*FileName);

	static uint32 ComputeAbsoluteIndex(int RelOrAbsIndex, uint32 CurrentAmount);

	uint32 FindOrCreateMaterialId(const std::string &MatName);

	//
	std::vector<COBJMeshMaterial>			Materials;
	// read from the OBJ
	std::wstring							MTLLibName;
};

inline OBJMeshLoader::EOBJLoaderFlags operator|(OBJMeshLoader::EOBJLoaderFlags a, OBJMeshLoader::EOBJLoaderFlags b)
{
	return (OBJMeshLoader::EOBJLoaderFlags)((uint32)a | (uint32)b);
}
