#pragma once

#include "DX12Lib/AppBase.h"
#include "DX12Lib/Mesh.h"


class App2 : public AppBase
{
public:
    using super = AppBase;

    App2(const std::wstring& name, int width, int height, bool vSync = false);

    virtual bool LoadContent() override;
    virtual void UnloadContent() override;

protected:

    virtual void OnRender(RenderEventArgs& e) override;
    virtual void OnResize(ResizeEventArgs& e) override; 

    Mesh mesh;
};