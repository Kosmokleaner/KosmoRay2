#include "App2.h"

#include "DX12Lib/Application.h"
#include "DX12Lib/CommandQueue.h"
#include "DX12Lib/Helpers.h"
#include "DX12Lib/Window.h"
#include "RelativeMouseInput.h"

#include "Mathlib.h" // clamp()
#include <Mock12.h>

#include <d3dcompiler.h>

#include <algorithm> // For std::min and std::max.



// todo: bad tangent and binormal, normal is smooth which looks bad for box
static VFormatFull g_Vertices[8] = {
    VFormatFull(glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(0,0,0), glm::vec3(0,0,0), glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec2(0.0f, 0.0f), 0), // 0
    VFormatFull(glm::vec3(-1.0f,  1.0f, -1.0f), glm::vec3(0,0,0), glm::vec3(0,0,0), glm::vec3(-1.0f,  1.0f, -1.0f), glm::vec2(0.0f, 1.0f), 0), // 1
    VFormatFull(glm::vec3(1.0f,  1.0f, -1.0f), glm::vec3(0,0,0), glm::vec3(0,0,0), glm::vec3(1.0f,  1.0f, -1.0f), glm::vec2(1.0f, 1.0f), 0), // 2
    VFormatFull(glm::vec3(1.0f, -1.0f, -1.0f), glm::vec3(0,0,0), glm::vec3(0,0,0), glm::vec3(1.0f, -1.0f, -1.0f), glm::vec2(1.0f, 0.0f), 0), // 3
    VFormatFull(glm::vec3(-1.0f, -1.0f,  1.0f), glm::vec3(0,0,0), glm::vec3(0,0,0), glm::vec3(-1.0f, -1.0f,  1.0f), glm::vec2(0.0f, 0.0f), 0), // 4
    VFormatFull(glm::vec3(-1.0f,  1.0f,  1.0f), glm::vec3(0,0,0), glm::vec3(0,0,0), glm::vec3(-1.0f,  1.0f,  1.0f), glm::vec2(0.0f, 1.0f), 0), // 5
    VFormatFull(glm::vec3(1.0f,  1.0f,  1.0f), glm::vec3(0,0,0), glm::vec3(0,0,0), glm::vec3(1.0f,  1.0f,  1.0f), glm::vec2(1.0f, 1.0f), 0), // 6
    VFormatFull(glm::vec3(1.0f, -1.0f,  1.0f), glm::vec3(0,0,0), glm::vec3(0,0,0), glm::vec3(1.0f, -1.0f,  1.0f), glm::vec2(1.0f, 0.0f), 0)  // 7
};

static Mesh::IndexType g_Indicies[36] =
{
    0, 1, 2, 0, 2, 3,
    4, 6, 5, 4, 7, 6,
    4, 5, 1, 4, 1, 0,
    3, 2, 6, 3, 6, 7,
    1, 5, 6, 1, 6, 2,
    4, 0, 3, 4, 3, 7
};

App2::App2(const std::wstring& name, int width, int height, bool vSync)
    : super(name, width, height, vSync)
{
}


bool App2::LoadContent()
{
    auto device = Application::Get().renderer.device;
    auto commandQueue = Application::Get().renderer.copyCommandQueue;
    auto commandList = commandQueue->GetCommandList();

//    Renderer renderer = Application::Get().renderer;
    Application::Get().renderer.copyCommandList = commandList.Get();

    mesh.startUpload(Application::Get().renderer, g_Vertices, _countof(g_Vertices), g_Indicies, _countof(g_Indicies));

    // Create the descriptor heap for the depth-stencil view.
    depthStencilDescriptorHeap.CreateDescriptorHeap(Application::Get().renderer, 1, D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

    // Load the vertex shader.
    ComPtr<ID3DBlob> vertexShaderBlob;
    ThrowIfFailed(D3DReadFileToBlob(L"VertexShader.cso", &vertexShaderBlob));

    // Load the pixel shader.
    ComPtr<ID3DBlob> pixelShaderBlob;
    ThrowIfFailed(D3DReadFileToBlob(L"PixelShader.cso", &pixelShaderBlob));

    // Create a root signature.
    D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
    featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
    if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
    {
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
    }

    // Allow input layout and deny unnecessary access to certain pipeline stages.
    D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

    // A single 32-bit constant root parameter that is used by the vertex shader.
    CD3DX12_ROOT_PARAMETER1 rootParameters[1];
    rootParameters[0].InitAsConstants(sizeof(glm::mat4) / 4, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
    rootSignatureDescription.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, rootSignatureFlags);

    // Serialize the root signature.
    ComPtr<ID3DBlob> rootSignatureBlob;
    ComPtr<ID3DBlob> errorBlob;
    ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDescription,
        featureData.HighestVersion, &rootSignatureBlob, &errorBlob));
    // Create the root signature.
    ThrowIfFailed(device->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(),
        rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature)));

    struct PipelineStateStream
    {
        CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
        CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT InputLayout;
        CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;
        CD3DX12_PIPELINE_STATE_STREAM_VS VS;
        CD3DX12_PIPELINE_STATE_STREAM_PS PS;
        CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT DSVFormat;
        CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
    } pipelineStateStream;

    D3D12_RT_FORMAT_ARRAY rtvFormats = {};
    rtvFormats.NumRenderTargets = 1;
    rtvFormats.RTFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

    pipelineStateStream.pRootSignature = rootSignature.Get();
    pipelineStateStream.InputLayout = { VFormatFull::GetDesc().Pointer, VFormatFull::GetDesc().Count };
    pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pipelineStateStream.VS = CD3DX12_SHADER_BYTECODE(vertexShaderBlob.Get());
    pipelineStateStream.PS = CD3DX12_SHADER_BYTECODE(pixelShaderBlob.Get());
    pipelineStateStream.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    pipelineStateStream.RTVFormats = rtvFormats;

    D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc = {
        sizeof(PipelineStateStream), &pipelineStateStream
    };
    ThrowIfFailed(device->CreatePipelineState(&pipelineStateStreamDesc, IID_PPV_ARGS(&pipelineState)));

    auto fenceValue = commandQueue->ExecuteCommandList(commandList);
    commandQueue->WaitForFenceValue(fenceValue);

    mesh.end();

    m_ContentLoaded = true;

    // Resize/Create the depth buffer.
    ResizeDepthBuffer(GetClientWidth(), GetClientHeight());

    return true;
}

void App2::OnResize(ResizeEventArgs& e)
{
    if (e.Width != GetClientWidth() || e.Height != GetClientHeight())
    {
        super::OnResize(e);

        m_Viewport = CD3DX12_VIEWPORT(0.0f, 0.0f,
            static_cast<float>(e.Width), static_cast<float>(e.Height));

        ResizeDepthBuffer(e.Width, e.Height);
    }
}

void App2::UnloadContent()
{
    m_ContentLoaded = false;
}


void App2::OnRender(RenderEventArgs& e)
{
    super::OnRender(e);

    auto commandQueue = Application::Get().renderer.directCommandQueue;
    auto commandList = commandQueue->GetCommandList();

    UINT currentBackBufferIndex = m_pWindow->GetCurrentBackBufferIndex();
    auto backBuffer = m_pWindow->GetCurrentBackBuffer();
    auto rtv = m_pWindow->GetCurrentRenderTargetView();
    auto dsv = depthStencilDescriptorHeap.descriptorHeap->GetCPUDescriptorHandleForHeapStart();

    // Clear the render targets.
    {
        TransitionResource(commandList, backBuffer,
            D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

        FLOAT clearColor[] = { 0.4f, 0.6f, 0.9f, 1.0f };

        ClearRTV(commandList, rtv, clearColor);
        ClearDepth(commandList, dsv);
    }

    commandList->SetPipelineState(pipelineState.Get());
    commandList->SetGraphicsRootSignature(rootSignature.Get());

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->IASetVertexBuffers(0, 1, &mesh.vertexBufferView);
    commandList->IASetIndexBuffer(&mesh.indexBufferView);

    commandList->RSSetViewports(1, &m_Viewport);
    commandList->RSSetScissorRects(1, &m_ScissorRect);

    commandList->OMSetRenderTargets(1, &rtv, FALSE, &dsv);

    // Update the MVP matrix

    // worldFromLocal * eyeFromWorld * clipFromEye

    glm::mat4 mvpMatrix = glm::transpose(m_ModelMatrix * m_ViewMatrix * m_ProjectionMatrix);

    commandList->SetGraphicsRoot32BitConstants(0, sizeof(glm::mat4) / 4, &mvpMatrix, 0);

    commandList->DrawIndexedInstanced(_countof(g_Indicies), 1, 0, 0, 0);

    // Present
    {
        TransitionResource(commandList, backBuffer,
            D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

        fenceValues[currentBackBufferIndex] = commandQueue->ExecuteCommandList(commandList);

        currentBackBufferIndex = m_pWindow->Present();

        commandQueue->WaitForFenceValue(fenceValues[currentBackBufferIndex]);
    }
}
