#include "AppBase.h"
#include "Application.h"
#include "Helpers.h"
#include "../RelativeMouseInput.h"




void AppBase::ResizeDepthBuffer(int width, int height)
{
    if (m_ContentLoaded)
    {
        // Flush any GPU commands that might be referencing the depth buffer.
        Application::Get().renderer.Flush();

        width = std::max(1, width);
        height = std::max(1, height);

        auto device = Application::Get().renderer.device;

        // Resize screen dependent resources.
        // Create a depth buffer.
        D3D12_CLEAR_VALUE optimizedClearValue = {};
        optimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
        optimizedClearValue.DepthStencil = { 1.0f, 0 };

        CD3DX12_HEAP_PROPERTIES a(D3D12_HEAP_TYPE_DEFAULT);
        CD3DX12_RESOURCE_DESC b = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, width, height,
            1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

        ThrowIfFailed(device->CreateCommittedResource(
            &a,
            D3D12_HEAP_FLAG_NONE,
            &b,
            D3D12_RESOURCE_STATE_DEPTH_WRITE,
            &optimizedClearValue,
            IID_PPV_ARGS(&depthBuffer)
        ));

        // Update the depth-stencil view.
        D3D12_DEPTH_STENCIL_VIEW_DESC dsv = {};
        dsv.Format = DXGI_FORMAT_D32_FLOAT;
        dsv.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        dsv.Texture2D.MipSlice = 0;
        dsv.Flags = D3D12_DSV_FLAG_NONE;

        device->CreateDepthStencilView(depthBuffer.Get(), &dsv,
            depthStencilDescriptorHeap.descriptorHeap->GetCPUDescriptorHandleForHeapStart());
    }
}


void AppBase::TransitionResource(ComPtr<ID3D12GraphicsCommandList2> commandList,
    ComPtr<ID3D12Resource> resource,
    D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState)
{
    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        resource.Get(),
        beforeState, afterState);

    commandList->ResourceBarrier(1, &barrier);
}

void AppBase::ClearRTV(ComPtr<ID3D12GraphicsCommandList2> commandList,
    D3D12_CPU_DESCRIPTOR_HANDLE rtv, FLOAT* clearColor)
{
    commandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
}

void AppBase::ClearDepth(ComPtr<ID3D12GraphicsCommandList2> commandList,
    D3D12_CPU_DESCRIPTOR_HANDLE dsv, FLOAT depth)
{
    commandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH, depth, 0, 0, nullptr);
}

void AppBase::OnUpdate(UpdateEventArgs& e)
{
    static uint64_t frameCount = 0;
    static double totalTime = 0.0;

    super::OnUpdate(e);

    totalTime += e.ElapsedTime;
    frameCount++;

    if (totalTime > 1.0)
    {
        double fps = frameCount / totalTime;

//        char buffer[512];
//        sprintf_s(buffer, "FPS: %f\n", fps);
//        OutputDebugStringA(buffer);

        frameCount = 0;
        totalTime = 0.0;
    }

    // Update the model matrix.
//    float angle = static_cast<float>(e.TotalTime * 90.0);
//    float angle = 0.0f;
//    const glm::vec4 rotationAxis(0, 1, 1, 0);
//    m_ModelMatrix = glm::transpose(XMMatrixRotationAxis(rotationAxis, ConvertToRadians(angle)));
    m_ModelMatrix = glm::mat4(1);

    // Update the view matrix.
    const glm::vec3 eyePosition(0, 0, -10);
    const glm::vec3 focusPoint(0, 0, 0);
    const glm::vec3 upDirection(0, 1, 0);
    m_ViewMatrix = glm::transpose(glm::lookAtLH(eyePosition, focusPoint, upDirection));

    extern CRelativeMouseInput g_MouseInput;
    CRelativeMouseInput::MouseInputButtons Buttons = g_MouseInput.GetMouseButtons();

    // right mouse button
    if (Buttons & 2)
    {
        g_MouseInput.SetUserCapture("RightMouseButton");
    }
    else
    {
        g_MouseInput.ResetUserCapture("RightMouseButton");
    }

    CRelativeMouseInput::MouseInputData data = g_MouseInput.ClaimMouseInputData("RightMouseButton");

    static bool first = true;
    if (first) {
//        camera.SetPos(glm::vec3(0, 2, -10));
//        camera.SetRotation(glm::vec2(3.1415f, 0.0f));
        // look at cornell box
		camera.SetPos(glm::vec3(0, 1, 3.5f));
		camera.SetRotation(glm::vec2(0, 0.0f));
        first = false;
    }

    //				const float rotateSpeed = 2 * InCamera.GetVerticalFov() / g_Renderer.GetHeight();
    const float rotateSpeed = 1 * 0.002f;		// todo
    const float movementSpeed = 50.0f / 10.0f;

    if (data.IsValid())
    {
        float fInvMouse = 1.0;
        camera.Rotate(rotateSpeed * data.RelativeX, -rotateSpeed * data.RelativeY * fInvMouse);
    }

    glm::vec3 forward = camera.GetForward();
    glm::vec3 left = glm::normalize(glm::cross(forward, camera.GetUp()));

    float dt = (float)e.ElapsedTime;
    forward *= movementSpeed * dt;
    left *= movementSpeed * dt;

    glm::vec3 move(0, 0, 0);

    if (m_pWindow->isActive())
    {
        if (GetAsyncKeyState('A'))
        {
            move += left;
        }
        else if (GetAsyncKeyState('D'))
        {
            move -= left;
        }

        if (GetAsyncKeyState('W'))
        {
            move += forward;
        }
        else if (GetAsyncKeyState('S'))
        {
            move -= forward;
        }
    }
    camera.Move(move);

    // world->eye aka eyeFromWorld
    //camera.GetDirX();
    //m_ViewMatrix = glm::inverse(0, camera.GetViewMatrix());
    m_ViewMatrix = glm::transpose(camera.GetViewMatrix());

    // Update the projection matrix.
//    float aspectRatio = GetClientWidth() / static_cast<float>(GetClientHeight());
    // XMMatrixPerspectiveFovLH
    m_ProjectionMatrix = glm::transpose(glm::perspectiveFovLH<float>(fieldOfView * PI / 180, (float)GetClientWidth(), (float)GetClientHeight(), 0.1f, 100.0f));
}



void AppBase::OnKeyPressed(KeyEventArgs& e)
{
    super::OnKeyPressed(e);

    switch (e.Key)
    {
    case KeyCode::Escape:
        Application::Get().Quit(0);
        break;

    case KeyCode::Enter:
        break;

    case KeyCode::F11:
        if (e.Alt)
            m_pWindow->SetFullscreen(m_pWindow->IsFullScreen());
        break;

    case KeyCode::V:
        m_pWindow->SetVSync(m_pWindow->IsVSync());
        break;
    }
}

void AppBase::OnMouseWheel(MouseWheelEventArgs& e)
{
/*    fieldOfView -= e.WheelDelta;
    fieldOfView = clamp(fieldOfView, 12.0f, 90.0f);

    char buffer[256];
    sprintf_s(buffer, "FoV: %f\n", fieldOfView);
    OutputDebugStringA(buffer);
*/
}

void AppBase::OnMouseMoved(MouseMotionEventArgs& e)
{
    mouseXY = glm::ivec2(e.X, e.Y);
}
