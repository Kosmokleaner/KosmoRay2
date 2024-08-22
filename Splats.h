#pragma once
#include "Renderer.h"

// per splat data
#pragma pack(push,1)
struct SplatData
{
	SplatData()
	{
	}

	glm::vec3 position;
	float radius;
};
#pragma pack(pop)


class Splats
{
public:
    // for ray tracing
    ComPtr<ID3D12Resource> bottomLevelAccelerationStructure;

	void CreateRenderMesh(Renderer& renderer);

	// build bottomLevelAccelerationStructure
    void BuildAccelerationStructures(Renderer& renderer);

	UINT CreateSRVs(Renderer& renderer);
	void Reset();

	std::vector<SplatData> splatData;

	D3DBuffer splatBuffer;
	D3DBuffer aabbBuffer;

private:
};

