#pragma once
#include "Game.h"
#include "Window.h"
#include "Camera.h"
#include "Mesh.h"
#include "DescriptorHeap.h"

#include "Mathlib.h"

#include <d3d12.h>
#undef min
#undef max

#include <d3dx12.h>

#include <wrl.h> // ComPtr<>
using namespace Microsoft::WRL;


class AppBase : public Game
{
public:
    using super = Game;

    AppBase(const std::wstring& name, int width, int height, bool vSync)
        : Game(name, width, height, vSync), m_ScissorRect(CD3DX12_RECT(0, 0, LONG_MAX, LONG_MAX))
        , m_Viewport(CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)))
        , m_ContentLoaded(false)
    {
    }

protected:

    virtual void OnUpdate(UpdateEventArgs& e) override;
    virtual void OnKeyPressed(KeyEventArgs& e) override;
    virtual void OnMouseWheel(MouseWheelEventArgs& e) override;

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
        D3D12_CPU_DESCRIPTOR_HANDLE dsv, FLOAT depth = 1.0f);

    // Resize the depth buffer to match the size of the client area.
    void ResizeDepthBuffer(int width, int height);

    // ---------------------------------

    uint64_t fenceValues[Window::BufferCount] = {};

    // Depth buffer
    ComPtr<ID3D12Resource> depthBuffer;
    // Descriptor heap for depth buffer
    DescriptorHeap depthStencilDescriptorHeap;

    // Root signatures
    ComPtr<ID3D12RootSignature> rootSignature;

    // Pipeline state object.
    ComPtr<ID3D12PipelineState> pipelineState;

    D3D12_VIEWPORT m_Viewport;
    D3D12_RECT m_ScissorRect;

    // in degree, fovAngleY for XMMatrixPerspectiveFovLH
    float fieldOfView = 45.0f;

    // local->world aka worldFromLocal
    DirectX::XMMATRIX m_ModelMatrix = {};
    // world->eye aka eyeFromWorld
    DirectX::XMMATRIX m_ViewMatrix = {};
    // eye->clip aka clipFromEye
    DirectX::XMMATRIX m_ProjectionMatrix = {};

    CTransform camera;

    bool m_ContentLoaded = false;
};

