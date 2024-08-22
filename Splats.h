#pragma once
#include "Renderer.h"

// per splat data
struct SplatData
{
	SplatData()
	{
	}

    glm::vec3 Pos;
	float radius;
//    glm::vec3 TangentN;
//    glm::vec2 UV;
};


class Splats
{
public:
    // for ray tracing
    ComPtr<ID3D12Resource> bottomLevelAccelerationStructure;

	void CreateRenderMesh(Renderer& renderer);

	// build bottomLevelAccelerationStructure
    void BuildAccelerationStructures(Renderer& renderer);

	void Reset();

	std::vector<SplatData> splatData;

	D3DBuffer splatBuffer;
	D3DBuffer aabbBuffer;
	D3D12_VERTEX_BUFFER_VIEW splatBufferView;

private:

	void init();
};

