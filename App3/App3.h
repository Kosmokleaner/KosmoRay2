#pragma once

#include "DX12Lib/AppBase.h"
#include "DX12Lib/Helpers.h" // ConstantBuffer<>
#include "../DataBlock.h"

struct Viewport
{
    float left;
    float top;
    float right;
    float bottom;
};

struct RayGenConstantBuffer
{
    Viewport viewport;
    Viewport stencil;
};

struct SceneConstantBuffer
{
    XMMATRIX clipFromWorld;
    XMMATRIX worldFromClip;
    XMFLOAT4 cameraPosition;
    // .x:frac(time), y.:frac(time*0.1)
    XMFLOAT4 sceneParam0;
    uint32 raytraceFlags;
    uint32 dummy[3];
};

class App3 : public AppBase
{
public:
    using super = AppBase;

    App3(const std::wstring& name, int width, int height, bool vSync = false);
    ~App3();

    virtual bool LoadContent() override;
    virtual void UnloadContent() override;

protected:

    virtual void OnUpdate(UpdateEventArgs& e) override;
    virtual void OnRender(RenderEventArgs& e) override;
    virtual void OnKeyPressed(KeyEventArgs& e) override;
    virtual void OnResize(ResizeEventArgs& e) override; 

private:
    
    ComPtr<ID3D12RootSignature> m_raytracingGlobalRootSignature;
    ComPtr<ID3D12RootSignature> m_raytracingLocalRootSignature;

    ConstantBuffer<SceneConstantBuffer> m_sceneCB;

    // RAY_FLAG_FORCE_NON_OPAQUE | RAY_FLAG_SKIP_CLOSEST_HIT_SHADER
    uint raytraceFlags = 0x2 | 0x8;

    // ray tracing ----------------------------------

    void CreateDeviceDependentResources();
    void CreateRootSignatures();
    void CreateRaytracingPipelineStateObject();
    void BuildAccelerationStructures();
    // Build shader tables, which define shaders and their local root arguments
    void BuildShaderTables();
    void CreateRaytracingOutputResource();
    void CreateLocalRootSignatureSubobjects(CD3DX12_STATE_OBJECT_DESC* raytracingPipeline);
    void SerializeAndCreateRaytracingRootSignature(D3D12_ROOT_SIGNATURE_DESC& desc, ComPtr<ID3D12RootSignature>* rootSig);
    void ReleaseDeviceDependentResources();
    void DoRaytracing(ComPtr<ID3D12GraphicsCommandList2> commandList, UINT currentBackBufferIndex);
    void CopyRaytracingOutputToBackbuffer(ComPtr<ID3D12GraphicsCommandList2> commandList);
    void CreateWindowSizeDependentResources();
    void UpdateForSizeChange(UINT width, UINT height);

    // DirectX Raytracing (DXR) attributes
    ComPtr<ID3D12StateObject> dxrStateObject;

    // Raytracing scene
    RayGenConstantBuffer rayGenCB;

    Mesh meshA;
    Mesh meshB;

    D3D12_GPU_DESCRIPTOR_HANDLE m_allIB = {};
    D3D12_GPU_DESCRIPTOR_HANDLE m_allVB = {};

    // Acceleration structure
    ComPtr<ID3D12Resource> topLevelAccelerationStructure;

    // Raytracing output, is copied to backbuffer each frame
    DataBlock m_raytracingOutput;

    // Shader tables
    ComPtr<ID3D12Resource> missShaderTable;
    ComPtr<ID3D12Resource> hitGroupShaderTable;
    ComPtr<ID3D12Resource> rayGenShaderTable;
};