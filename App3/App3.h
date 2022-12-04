#pragma once

#include "DX12Lib/Game.h"
#include "DX12Lib/Window.h"
#include "DX12Lib/Helpers.h" // ConstantBuffer<>
#include "Camera.h"

#include <DirectXMath.h>
#include <d3dx12.h>
using namespace DirectX; // XMFLOAT4

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

struct D3DBuffer
{
    ComPtr<ID3D12Resource> resource;
    D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptorHandle;
    D3D12_GPU_DESCRIPTOR_HANDLE gpuDescriptorHandle;
};

class App3 : public Game
{
public:
    using super = Game;

    CTransform camera;

    App3(const std::wstring& name, int width, int height, bool vSync = false);
    ~App3();
    /**
     *  Load content required for the demo.
     */
    virtual bool LoadContent() override;

    /**
     *  Unload demo specific content that was loaded in LoadContent.
     */
    virtual void UnloadContent() override;
protected:
    /**
     *  Update the game logic.
     */
    virtual void OnUpdate(UpdateEventArgs& e) override;

    /**
     *  Render stuff.
     */
    virtual void OnRender(RenderEventArgs& e) override;

    /**
     * Invoked by the registered window when a key is pressed
     * while the window has focus.
     */
    virtual void OnKeyPressed(KeyEventArgs& e) override;

    /**
     * Invoked when the mouse wheel is scrolled while the registered window has focus.
     */
    virtual void OnMouseWheel(MouseWheelEventArgs& e) override;


    virtual void OnResize(ResizeEventArgs& e) override; 

private:
    // Helper functions
    // Transition a resource
    void TransitionResource(ComPtr<ID3D12GraphicsCommandList2> commandList,
        ComPtr<ID3D12Resource> resource,
        D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState);

    // Clear a render target view.
    void ClearRTV(ComPtr<ID3D12GraphicsCommandList2> commandList,
        D3D12_CPU_DESCRIPTOR_HANDLE rtv, FLOAT* clearColor);

    // Clear the depth of a depth-stencil view.
    void ClearDepth(ComPtr<ID3D12GraphicsCommandList2> commandList,
        D3D12_CPU_DESCRIPTOR_HANDLE dsv, FLOAT depth = 1.0f );

    // Create a GPU buffer.
    void UpdateBufferResource(ComPtr<ID3D12GraphicsCommandList2> commandList,
        ID3D12Resource** pDestinationResource, ID3D12Resource** pIntermediateResource,
        size_t numElements, size_t elementSize, const void* bufferData, 
        D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE );

    // Resize the depth buffer to match the size of the client area.
    void ResizeDepthBuffer(int width, int height);
    
    uint64_t m_FenceValues[Window::BufferCount] = {};

    // cube, todo: remove
    ComPtr<ID3D12Resource> m_VertexBuffer;
    // cube, todo: remove
    D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;
    // cube, todo: remove
    ComPtr<ID3D12Resource> m_IndexBuffer;
    // cube, todo: remove
    D3D12_INDEX_BUFFER_VIEW m_IndexBufferView;

    // Depth buffer.
    ComPtr<ID3D12Resource> m_DepthBuffer;
    // Descriptor heap for depth buffer.
    ComPtr<ID3D12DescriptorHeap> m_DSVHeap;

    // Root signatures
    ComPtr<ID3D12RootSignature> m_RootSignature;
    ComPtr<ID3D12RootSignature> m_raytracingGlobalRootSignature;
    ComPtr<ID3D12RootSignature> m_raytracingLocalRootSignature;

    // Pipeline state object.
    ComPtr<ID3D12PipelineState> m_PipelineState;

    ConstantBuffer<SceneConstantBuffer> m_sceneCB;

    D3D12_VIEWPORT m_Viewport;
    D3D12_RECT m_ScissorRect;

    float m_FoV = 0.0f;

    // local->world aka worldFromLocal
    DirectX::XMMATRIX m_ModelMatrix;
    // world->eye aka eyeFromWorld
    DirectX::XMMATRIX m_ViewMatrix;
    // eye->clip aka clipFromEye
    DirectX::XMMATRIX m_ProjectionMatrix;

    // RAY_FLAG_FORCE_NON_OPAQUE | RAY_FLAG_SKIP_CLOSEST_HIT_SHADER
    uint raytraceFlags = 0x2 | 0x8;

    bool m_ContentLoaded = false;

    // ray tracing ----------------------------------

    void CreateDeviceDependentResources();
    void CreateRaytracingInterfaces();
    void CreateRootSignatures();
    void CreateRaytracingPipelineStateObject();
    void CreateDescriptorHeap();
    void BuildGeometry();
    void BuildAccelerationStructures();
    void BuildShaderTables();
    void CreateRaytracingOutputResource();
    void CreateLocalRootSignatureSubobjects(CD3DX12_STATE_OBJECT_DESC* raytracingPipeline);
    void SerializeAndCreateRaytracingRootSignature(D3D12_ROOT_SIGNATURE_DESC& desc, ComPtr<ID3D12RootSignature>* rootSig);
    void ReleaseDeviceDependentResources();
    UINT AllocateDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE* cpuDescriptor, UINT descriptorIndexToUse = UINT_MAX);
//    ID3D12GraphicsCommandList4* GetCommandList() const { return m_dxrCommandList.Get(); }
    void DoRaytracing(ComPtr<ID3D12GraphicsCommandList2> commandList, UINT currentBackBufferIndex);
    void CopyRaytracingOutputToBackbuffer(ComPtr<ID3D12GraphicsCommandList2> commandList);
    void CreateWindowSizeDependentResources();
    void UpdateForSizeChange(UINT width, UINT height);
    UINT CreateBufferSRV(D3DBuffer* buffer, UINT numElements, UINT elementSize);

    // DirectX Raytracing (DXR) attributes
    ComPtr<ID3D12Device5> m_dxrDevice;
//    ComPtr<ID3D12GraphicsCommandList4> m_dxrCommandList;
    ComPtr<ID3D12StateObject> m_dxrStateObject;

    // Descriptors
    ComPtr<ID3D12DescriptorHeap> m_descriptorHeap;
    UINT m_descriptorsAllocated = 0;
    UINT m_descriptorSize = 0;

    // Raytracing scene
    RayGenConstantBuffer m_rayGenCB;

    // Geometry
    typedef UINT16 Index;
//    struct Vertex { float x, y, z; };
    // mesh
    D3DBuffer m_indexBuffer;
    // mesh
    D3DBuffer m_vertexBuffer;

    // Acceleration structure
    ComPtr<ID3D12Resource> m_accelerationStructure;
    ComPtr<ID3D12Resource> m_bottomLevelAccelerationStructure;
    ComPtr<ID3D12Resource> m_topLevelAccelerationStructure;

    // Raytracing output
    ComPtr<ID3D12Resource> m_raytracingOutput;
    D3D12_GPU_DESCRIPTOR_HANDLE m_raytracingOutputResourceUAVGpuDescriptor;
    UINT m_raytracingOutputResourceUAVDescriptorHeapIndex = UINT_MAX;

    // Shader tables
    ComPtr<ID3D12Resource> m_missShaderTable;
    ComPtr<ID3D12Resource> m_hitGroupShaderTable;
    ComPtr<ID3D12Resource> m_rayGenShaderTable;
};