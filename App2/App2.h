#pragma once

#include "DX12Lib/AppBase.h"


class App2 : public AppBase
{
public:
    using super = AppBase;

    App2(const std::wstring& name, int width, int height, bool vSync = false);

    virtual bool LoadContent() override;
    virtual void UnloadContent() override;

protected:

    virtual void OnRender(RenderEventArgs& e) override;
    virtual void OnKeyPressed(KeyEventArgs& e) override;
    virtual void OnMouseWheel(MouseWheelEventArgs& e) override;
    virtual void OnResize(ResizeEventArgs& e) override; 

    // Vertex buffer for the cube.
    ComPtr<ID3D12Resource> m_VertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;
    // Index buffer for the cube.
    ComPtr<ID3D12Resource> m_IndexBuffer;
    D3D12_INDEX_BUFFER_VIEW m_IndexBufferView;
};