#include "App3.h"

#include "DX12Lib/Application.h"
#include "DX12Lib/CommandQueue.h"
#include "DX12Lib/Helpers.h"
#include "DX12Lib/Window.h"
#include "CompiledShaders\Raytracing.hlsl.h" // g_pRaytracing
#include "RelativeMouseInput.h"
#include "external/nv-api/nvapi.h"

#include "Mathlib.h"
#include <Mock12.h>

#include <d3dcompiler.h> // D3DReadFileToBlob

// only on NVidia
bool g_NVAPI_enabled = false;

const wchar_t* c_hitGroupName = L"MyHitGroup";
const wchar_t* c_raygenShaderName = L"MyRaygenShader";
const wchar_t* c_closestHitShaderName = L"MyClosestHitShader";
const wchar_t* c_anyHitShaderName = L"MyAnyHitShader";
const wchar_t* c_missShaderName = L"MyMissShader";

namespace GlobalRootSignatureParams {
    enum Value {
        OutputViewSlot,             // DescriptorTable      UAV space0: u0(RenderTarget) space1: u0, u1 (Nvidia)
        FeedbackSlot,               // DescriptorTable      UAV space0: u1(g_Feedback)
        AccelerationStructureSlot,  // ShaderResourceView   SRV t0
        SceneConstant,              // ConstantBufferView   CBV b0
        IndexBuffer,                // DescriptorTable      SRV space100: t0
        VertexBuffer,               // DescriptorTable      SRV space101: t0 
        TextureSlot,                // DescriptorTable      SRV space102: t0
        Count
    };
}

namespace LocalRootSignatureParams {
    enum Value {
        ViewportConstantSlot = 0,
        Count
    };
}



App3::App3(const std::wstring& name, int width, int height, bool vSync)
    : super(name, width, height, vSync)
{
    rayGenCB.viewport = { -1.0f, -1.0f, 1.0f, 1.0f };
    UpdateForSizeChange(width, height);

    CreateDeviceDependentResources();
    CreateWindowSizeDependentResources();

    auto device = Application::Get().renderer.device;
    m_sceneCB.Create(device.Get(), Window::BufferCount, L"Scene Constant Buffer");
}

App3::~App3()
{
    ReleaseDeviceDependentResources();
}



void App3::CreateRootSignatures()
{
    // Global Root Signature
    // This is a root signature that is shared across all raytracing shaders invoked during a DispatchRays() call.
    {
        CD3DX12_DESCRIPTOR_RANGE UAVDescriptors[2];
        UAVDescriptors[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0);       // space 0: u0
        UAVDescriptors[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 2, 0, 1);       // space 1: u0 and u1
        CD3DX12_DESCRIPTOR_RANGE UAVFeedback;
        UAVFeedback.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1, 0);             // space 0: u1
        CD3DX12_DESCRIPTOR_RANGE SRVDescriptorIB[1];
        SRVDescriptorIB[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0, 101);    // space 101: t0: IndexBuffer, 2 SRV for 2 meshes
        CD3DX12_DESCRIPTOR_RANGE SRVDescriptorVB[1];
        SRVDescriptorVB[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0, 102);    // space 102: t0: VertexBuffer, 2 SRV for 2 meshes
        CD3DX12_DESCRIPTOR_RANGE SRVDescriptorTex[1];
        SRVDescriptorTex[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 103);    // space 103: t0: Texture

        CD3DX12_ROOT_PARAMETER rootParameters[GlobalRootSignatureParams::Count];
        rootParameters[GlobalRootSignatureParams::OutputViewSlot].InitAsDescriptorTable(ARRAYSIZE(UAVDescriptors), UAVDescriptors);
        rootParameters[GlobalRootSignatureParams::FeedbackSlot].InitAsDescriptorTable(1, &UAVFeedback);
        rootParameters[GlobalRootSignatureParams::AccelerationStructureSlot].InitAsShaderResourceView(0);   // 0 -> t0
        rootParameters[GlobalRootSignatureParams::SceneConstant].InitAsConstantBufferView(0);   // 0 -> b0
        rootParameters[GlobalRootSignatureParams::IndexBuffer].InitAsDescriptorTable(ARRAYSIZE(SRVDescriptorIB), SRVDescriptorIB);
        rootParameters[GlobalRootSignatureParams::VertexBuffer].InitAsDescriptorTable(ARRAYSIZE(SRVDescriptorVB), SRVDescriptorVB);
        rootParameters[GlobalRootSignatureParams::TextureSlot].InitAsDescriptorTable(ARRAYSIZE(SRVDescriptorTex), SRVDescriptorTex);

        CD3DX12_ROOT_SIGNATURE_DESC globalRootSignatureDesc(ARRAYSIZE(rootParameters), rootParameters);
        SerializeAndCreateRaytracingRootSignature(globalRootSignatureDesc, &m_raytracingGlobalRootSignature);
    }

    // Local Root Signature
    // This is a root signature that enables a shader to have unique arguments that come from shader tables.
    {
        CD3DX12_ROOT_PARAMETER rootParameters[LocalRootSignatureParams::Count];
        rootParameters[LocalRootSignatureParams::ViewportConstantSlot].InitAsConstants(SizeOfInUint32(rayGenCB), 1, 0);   // 1 -> b1
        CD3DX12_ROOT_SIGNATURE_DESC localRootSignatureDesc(ARRAYSIZE(rootParameters), rootParameters);
        localRootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;
        SerializeAndCreateRaytracingRootSignature(localRootSignatureDesc, &m_raytracingLocalRootSignature);
    }
}

bool App3::LoadContent()
{
    auto device = Application::Get().renderer.device;
    auto commandQueue = Application::Get().renderer.copyCommandQueue;
    auto commandList = commandQueue->GetCommandList();

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
    rootParameters[0].InitAsConstants(sizeof(XMMATRIX) / 4, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);

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

    m_ContentLoaded = true;

    // Resize/Create the depth buffer.
    ResizeDepthBuffer(GetClientWidth(), GetClientHeight());

    return true;
}

void App3::OnResize(ResizeEventArgs& e)
{
    if (e.Width != GetClientWidth() || e.Height != GetClientHeight())
    {
        super::OnResize(e);

        m_Viewport = CD3DX12_VIEWPORT(0.0f, 0.0f,
            static_cast<float>(e.Width), static_cast<float>(e.Height));

        ResizeDepthBuffer(e.Width, e.Height);
        CreateWindowSizeDependentResources();
    }
}

void App3::UnloadContent()
{
    m_ContentLoaded = false;
}

void App3::OnUpdate(UpdateEventArgs& e)
{
    super::OnUpdate(e);

    m_sceneCB->sceneParam0.x = frac(m_sceneCB->sceneParam0.x + (float)e.ElapsedTime);
    m_sceneCB->sceneParam0.y = frac(m_sceneCB->sceneParam0.y + (float)e.ElapsedTime * 0.1f);
    m_sceneCB->raytraceFlags = raytraceFlags;

    static uint32 FrameIndex = 0; ++FrameIndex;
    m_sceneCB->FrameIndex = FrameIndex;

    {
        // clipFromWorld = clipFromEye * eyeFromWorld
        XMMATRIX clipFromWorld = m_ProjectionMatrix * m_ViewMatrix;
        m_sceneCB->cameraPosition = XMFLOAT4(camera.GetPos().x, camera.GetPos().y, camera.GetPos().z, 0.0f);
        m_sceneCB->clipFromWorld = XMMatrixTranspose(clipFromWorld);
        m_sceneCB->worldFromClip = XMMatrixTranspose(XMMatrixInverse(nullptr, clipFromWorld));
    }
}


void App3::DoRaytracing(ComPtr<ID3D12GraphicsCommandList2> commandList, UINT currentBackBufferIndex)
{
    auto& renderer = Application::Get().renderer;

    commandList->SetComputeRootSignature(m_raytracingGlobalRootSignature.Get());

    // Copy dynamic buffers to GPU.
    {
        m_sceneCB.CopyStagingToGpu(currentBackBufferIndex);
        commandList->SetComputeRootConstantBufferView(GlobalRootSignatureParams::SceneConstant, m_sceneCB.GpuVirtualAddress(currentBackBufferIndex));
    }

    // Bind the heaps, acceleration structure and dispatch rays
    D3D12_DISPATCH_RAYS_DESC dispatchDesc = {};
    commandList->SetDescriptorHeaps(1, renderer.descriptorHeap.descriptorHeap.GetAddressOf());
    commandList->SetComputeRootDescriptorTable(GlobalRootSignatureParams::OutputViewSlot, m_raytracingOutput.m_UAVGpuDescriptor);
    commandList->SetComputeRootDescriptorTable(GlobalRootSignatureParams::FeedbackSlot, m_raytracingFeedback.m_UAVGpuDescriptor);
    commandList->SetComputeRootShaderResourceView(GlobalRootSignatureParams::AccelerationStructureSlot, topLevelAccelerationStructure->GetGPUVirtualAddress());

    // Set index and successive vertex buffer decriptor tables
//    commandList->SetComputeRootDescriptorTable(GlobalRootSignatureParams::IndexAndVertexBuffer, meshA.indexBuffer.gpuDescriptorHandle);
//    commandList->SetComputeRootDescriptorTable(GlobalRootSignatureParams::IndexAndVertexBuffer, meshB.indexBuffer.gpuDescriptorHandle);

    commandList->SetComputeRootDescriptorTable(GlobalRootSignatureParams::IndexBuffer, m_allIB);
    commandList->SetComputeRootDescriptorTable(GlobalRootSignatureParams::VertexBuffer, m_allVB);

    commandList->SetComputeRootDescriptorTable(GlobalRootSignatureParams::TextureSlot, m_texture.m_UAVGpuDescriptor);

    // hack
    ComPtr<ID3D12GraphicsCommandList4> m_dxrCommandList;
    ThrowIfFailed(commandList->QueryInterface(IID_PPV_ARGS(&m_dxrCommandList)), L"Couldn't get DirectX Raytracing interface for the command list.\n");

    // DispatchRays
    {
        auto* commandList = m_dxrCommandList.Get();
        auto* stateObject = dxrStateObject.Get();

        // Since each shader table has only one shader record, the stride is same as the size.
        dispatchDesc.HitGroupTable.StartAddress = hitGroupShaderTable->GetGPUVirtualAddress();
        dispatchDesc.HitGroupTable.SizeInBytes = hitGroupShaderTable->GetDesc().Width;
        dispatchDesc.HitGroupTable.StrideInBytes = dispatchDesc.HitGroupTable.SizeInBytes;
        dispatchDesc.MissShaderTable.StartAddress = missShaderTable->GetGPUVirtualAddress();
        dispatchDesc.MissShaderTable.SizeInBytes = missShaderTable->GetDesc().Width;
        dispatchDesc.MissShaderTable.StrideInBytes = dispatchDesc.MissShaderTable.SizeInBytes;
        dispatchDesc.RayGenerationShaderRecord.StartAddress = rayGenShaderTable->GetGPUVirtualAddress();
        dispatchDesc.RayGenerationShaderRecord.SizeInBytes = rayGenShaderTable->GetDesc().Width;
        dispatchDesc.Width = GetClientWidth();
        dispatchDesc.Height = GetClientHeight();
        dispatchDesc.Depth = 1;
        commandList->SetPipelineState1(stateObject);
        commandList->DispatchRays(&dispatchDesc);
    }
}

void App3::CopyRaytracingOutputToBackbuffer(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    auto renderTarget = m_pWindow->GetCurrentBackBuffer();

    {
        // D3D12_RESOURCE_UAV_BARRIER 

        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
        barrier.UAV.pResource = m_raytracingFeedback.m_resource.Get();
        commandList->ResourceBarrier(1, &barrier);
    }

    D3D12_RESOURCE_BARRIER preCopyBarriers[2];
    preCopyBarriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(renderTarget.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
//    preCopyBarriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(renderTarget.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_DEST);
    preCopyBarriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(m_raytracingOutput.m_resource.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
    commandList->ResourceBarrier(ARRAYSIZE(preCopyBarriers), preCopyBarriers);

    commandList->CopyResource(renderTarget.Get(), m_raytracingOutput.m_resource.Get());

    D3D12_RESOURCE_BARRIER postCopyBarriers[2];
    postCopyBarriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(renderTarget.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PRESENT);
    postCopyBarriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(m_raytracingOutput.m_resource.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

    commandList->ResourceBarrier(ARRAYSIZE(postCopyBarriers), postCopyBarriers);
}

void App3::OnRender(RenderEventArgs& e)
{
    super::OnRender(e);

    auto commandQueue = Application::Get().renderer.directCommandQueue;
    auto commandList = commandQueue->GetCommandList();

    UINT currentBackBufferIndex = m_pWindow->GetCurrentBackBufferIndex();
    auto backBuffer = m_pWindow->GetCurrentBackBuffer();

    {
        DoRaytracing(commandList, currentBackBufferIndex);
        CopyRaytracingOutputToBackbuffer(commandList);

        fenceValues[currentBackBufferIndex] = commandQueue->ExecuteCommandList(commandList);
        currentBackBufferIndex = m_pWindow->Present();
        commandQueue->WaitForFenceValue(fenceValues[currentBackBufferIndex]);
    }
}

void App3::OnKeyPressed(KeyEventArgs& e)
{
    super::OnKeyPressed(e);

    bool log = false;

    switch (e.Key)
    {
        case KeyCode::PageUp:
            --raytraceFlags;
            log = true;
            break;

        case KeyCode::PageDown:
            ++raytraceFlags;
            log = true;
            break;
    }

    if(log)
    {
        char buffer[512];
        sprintf_s(buffer, "raytraceFlags: 0x%x\n", raytraceFlags);
        OutputDebugStringA(buffer);
    }
}

void App3::SerializeAndCreateRaytracingRootSignature(D3D12_ROOT_SIGNATURE_DESC& desc, ComPtr<ID3D12RootSignature>* rootSig)
{
    auto device = Application::Get().renderer.device;
    ComPtr<ID3DBlob> blob;
    ComPtr<ID3DBlob> error;

    ThrowIfFailed(D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &error), error ? static_cast<wchar_t*>(error->GetBufferPointer()) : nullptr);
    ThrowIfFailed(device->CreateRootSignature(1, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&(*rootSig))));
}

// Update the application state with the new resolution.
void App3::UpdateForSizeChange(UINT width, UINT height)
{
//    DXSample::UpdateForSizeChange(width, height);
    float border = 0.1f;

    float aspectRatio = GetClientWidth() / static_cast<float>(GetClientHeight());

    if (GetClientWidth() <= GetClientHeight())
    {
        rayGenCB.stencil =
        {
            -1 + border, -1 + border * aspectRatio,
            1.0f - border, 1 - border * aspectRatio
        };
    }
    else
    {
        rayGenCB.stencil =
        {
            -1 + border / aspectRatio, -1 + border,
             1 - border / aspectRatio, 1.0f - border
        };

    }
}


// Local root signature and shader association
// This is a root signature that enables a shader to have unique arguments that come from shader tables.
void App3::CreateLocalRootSignatureSubobjects(CD3DX12_STATE_OBJECT_DESC* raytracingPipeline)
{
    // Hit group and miss shaders in this sample are not using a local root signature and thus one is not associated with them.

    // Local root signature to be used in a ray gen shader.
    {
        auto localRootSignature = raytracingPipeline->CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
        localRootSignature->SetRootSignature(m_raytracingLocalRootSignature.Get());
        // Shader association
        auto rootSignatureAssociation = raytracingPipeline->CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
        rootSignatureAssociation->SetSubobjectToAssociate(*localRootSignature);
        rootSignatureAssociation->AddExport(c_raygenShaderName);
    }
}

// Create a raytracing pipeline state object (RTPSO).
// An RTPSO represents a full set of shaders reachable by a DispatchRays() call,
// with all configuration options resolved, such as local signatures and other state.
void App3::CreateRaytracingPipelineStateObject()
{
    assert(Application::Get().renderer.dxrDevice);

    // Create 7 subobjects that combine into a RTPSO:
    // Subobjects need to be associated with DXIL exports (i.e. shaders) either by way of default or explicit associations.
    // Default association applies to every exported shader entrypoint that doesn't have any of the same type of subobject associated with it.
    // This simple sample utilizes default shader association except for local root signature subobject
    // which has an explicit association specified purely for demonstration purposes.
    // 1 - DXIL library
    // 1 - Triangle hit group
    // 1 - Shader config
    // 2 - Local root signature and association
    // 1 - Global root signature
    // 1 - Pipeline config
    CD3DX12_STATE_OBJECT_DESC raytracingPipeline{ D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE };


    // DXIL library
    // This contains the shaders and their entrypoints for the state object.
    // Since shaders are not considered a subobject, they need to be passed in via DXIL library subobjects.
    auto lib = raytracingPipeline.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
    D3D12_SHADER_BYTECODE libdxil = CD3DX12_SHADER_BYTECODE((void*)g_pRaytracing, ARRAYSIZE(g_pRaytracing));
    lib->SetDXILLibrary(&libdxil);
    // Define which shader exports to surface from the library.
    // If no shader exports are defined for a DXIL library subobject, all shaders will be surfaced.
    // In this sample, this could be omitted for convenience since the sample uses all shaders in the library. 
    {
        lib->DefineExport(c_raygenShaderName);
        lib->DefineExport(c_closestHitShaderName);
        lib->DefineExport(c_anyHitShaderName);
        lib->DefineExport(c_missShaderName);
    }

    // Triangle hit group
    // A hit group specifies closest hit, any hit and intersection shaders to be executed when a ray intersects the geometry's triangle/AABB.
    // In this sample, we only use triangle geometry with a closest hit shader, so others are not set.
    auto hitGroup = raytracingPipeline.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
    hitGroup->SetClosestHitShaderImport(c_closestHitShaderName);
    hitGroup->SetAnyHitShaderImport(c_anyHitShaderName);
    hitGroup->SetHitGroupExport(c_hitGroupName);
    hitGroup->SetHitGroupType(D3D12_HIT_GROUP_TYPE_TRIANGLES);

    // Shader config
    // Defines the maximum sizes in bytes for the ray payload and attribute structure.
    auto shaderConfig = raytracingPipeline.CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
    // see struct RayPayload
    UINT payloadSize = 4 * sizeof(float) + 3 * sizeof(float) + 4 + 4 + 4 + 4 + 4; // color + normal + prim + instance + minT + minTfront

    UINT attributeSize = 2 * sizeof(float); // float2 barycentrics
    shaderConfig->Config(payloadSize, attributeSize);

    // Local root signature and shader association
    CreateLocalRootSignatureSubobjects(&raytracingPipeline);
    // This is a root signature that enables a shader to have unique arguments that come from shader tables.

    // Global root signature
    // This is a root signature that is shared across all raytracing shaders invoked during a DispatchRays() call.
    auto globalRootSignature = raytracingPipeline.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();
    globalRootSignature->SetRootSignature(m_raytracingGlobalRootSignature.Get());

    // Pipeline config
    // Defines the maximum TraceRay() recursion depth.
    auto pipelineConfig = raytracingPipeline.CreateSubobject<CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();
    // PERFOMANCE TIP: Set max recursion depth as low as needed 
    // as drivers may apply optimization strategies for low recursion depths. 
    UINT maxRecursionDepth = 1; // ~ primary rays only. 
//    UINT maxRecursionDepth = 20;
    pipelineConfig->Config(maxRecursionDepth);

#if _DEBUG
    PrintStateObjectDesc(raytracingPipeline);
#endif

    if(g_NVAPI_enabled)
    {
        NvAPI_Status NvapiStatus = NvAPI_D3D12_SetNvShaderExtnSlotSpace(Application::Get().renderer.dxrDevice.Get(), 1, 1);
        if (NvapiStatus != NVAPI_OK)
        {
            assert(0);
        }
    }

    // Create the state object.
    ThrowIfFailed(Application::Get().renderer.dxrDevice->CreateStateObject(raytracingPipeline, IID_PPV_ARGS(&dxrStateObject)), L"Couldn't create DirectX Raytracing state object.\n");

    if (g_NVAPI_enabled)
    {
        // Disable the NVAPI extension slot again after state object creation.
        NvAPI_Status NvapiStatus = NvAPI_D3D12_SetNvShaderExtnSlotSpace(Application::Get().renderer.dxrDevice.Get(), ~0u, 1);
        if (NvapiStatus != NVAPI_OK)
        {
            assert(0);
        }
    }
}

void App3::BuildAccelerationStructures()
{
    meshA.BuildAccelerationStructures(Application::Get().renderer);
    meshB.BuildAccelerationStructures(Application::Get().renderer);

    auto device = Application::Get().renderer.device;

    auto commandQueue = Application::Get().renderer.directCommandQueue;
    assert(commandQueue);
    auto commandList = commandQueue->GetCommandList();

    // debug
    D3D12_COMMAND_QUEUE_DESC desc = commandQueue->GetD3D12CommandQueue()->GetDesc();
    assert(desc.Type != D3D12_COMMAND_LIST_TYPE_COPY);

    // Reset the command list for the acceleration structure construction.
//not needed?    commandList->Reset(commandAllocator, nullptr);

    // Get required sizes for an acceleration structure.
    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS topLevelInputs = {};
    topLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
    topLevelInputs.Flags = buildFlags;
    topLevelInputs.NumDescs = 1;
    topLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO topLevelPrebuildInfo = {};
    Application::Get().renderer.dxrDevice->GetRaytracingAccelerationStructurePrebuildInfo(&topLevelInputs, &topLevelPrebuildInfo);
    ThrowIfFalse(topLevelPrebuildInfo.ResultDataMaxSizeInBytes > 0);

    ComPtr<ID3D12Resource> scratchResource;
    AllocateUAVBuffer(device.Get(), commandList.Get(), topLevelPrebuildInfo.ScratchDataSizeInBytes, &scratchResource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, L"ScratchResource");

    // Allocate resources for acceleration structures.
    // Acceleration structures can only be placed in resources that are created in the default heap (or custom heap equivalent). 
    // Default heap is OK since the application doesn’t need CPU read/write access to them. 
    // The resources that will contain acceleration structures must be created in the state D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, 
    // and must have resource flag D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS. The ALLOW_UNORDERED_ACCESS requirement simply acknowledges both: 
    //  - the system will be doing this type of access in its implementation of acceleration structure builds behind the scenes.
    //  - from the app point of view, synchronization of writes/reads to acceleration structures is accomplished using UAV barriers.
    {
        D3D12_RESOURCE_STATES initialResourceState = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;

        AllocateUAVBuffer(device.Get(), commandList.Get(), topLevelPrebuildInfo.ResultDataMaxSizeInBytes, &topLevelAccelerationStructure, initialResourceState, L"TLAS");
    }

    // Create an instance desc for the bottom-level acceleration structure.
    ComPtr<ID3D12Resource> instanceDescs;

    uint32 instanceCount;
    {
        D3D12_RAYTRACING_INSTANCE_DESC instanceDesc[4] = {};

        instanceCount = _countof(instanceDesc);

        D3D12_RAYTRACING_INSTANCE_DESC *dst = instanceDesc;

        for(UINT i = 0; i < 4; ++i)
        {
            *dst = {};

            float size = 0.3f;

            // x: right, y:up (closer), z:behind
            if (i)
            {
                size = 1.0f;
                dst->Transform[i - 1][3] = (i == 2) ? 1.0f : 5.0f; 
            }

            dst->Transform[0][0] = dst->Transform[1][1] = dst->Transform[2][2] = size;
            dst->InstanceMask = 1;

            const uint32 meshIndex = (i == 0) ? 0 : 1;

            Mesh& ref = meshIndex ? meshB : meshA;

            dst->AccelerationStructure = ref.bottomLevelAccelerationStructure->GetGPUVirtualAddress();
            // will go to HLSL InstanceID()
            dst->InstanceID = meshIndex;

            ++dst;
        }

        AllocateUploadBuffer(device.Get(), instanceDesc, sizeof(instanceDesc), &instanceDescs, L"InstanceDescs");
    }

    // Top Level Acceleration Structure desc
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC topLevelBuildDesc = {};
    {
        topLevelInputs.InstanceDescs = instanceDescs->GetGPUVirtualAddress();
        topLevelInputs.NumDescs = instanceCount;
        topLevelBuildDesc.Inputs = topLevelInputs;
        topLevelBuildDesc.DestAccelerationStructureData = topLevelAccelerationStructure->GetGPUVirtualAddress();
        topLevelBuildDesc.ScratchAccelerationStructureData = scratchResource->GetGPUVirtualAddress();
    }

    // Build acceleration structure.
    // hack
    ComPtr<ID3D12GraphicsCommandList4> m_dxrCommandList;
    ThrowIfFailed(commandList->QueryInterface(IID_PPV_ARGS(&m_dxrCommandList)), L"Couldn't get DirectX Raytracing interface for the command list.\n");
    // BuildAccelerationStructure
    {
        auto* raytracingCommandList = m_dxrCommandList.Get();
        raytracingCommandList->BuildRaytracingAccelerationStructure(&topLevelBuildDesc, 0, nullptr);
    }

    // Kick off acceleration structure construction.
    auto fenceValue = commandQueue->ExecuteCommandList(commandList);
    commandQueue->WaitForFenceValue(fenceValue);
}

void App3::CreateWindowSizeDependentResources()
{
    CreateRaytracingOutputResource();

    // For simplicity, we will rebuild the shader tables.
    BuildShaderTables();
}

// This encapsulates all shader records - shaders and the arguments for their local root signatures.
void App3::BuildShaderTables()
{
    auto device = Application::Get().renderer.device;

    // Get shader identifiers.    
    ComPtr<ID3D12StateObjectProperties> stateObjectProperties;
    ThrowIfFailed(dxrStateObject.As(&stateObjectProperties));
    void* rayGenShaderIdentifier = stateObjectProperties.Get()->GetShaderIdentifier(c_raygenShaderName);
    void* missShaderIdentifier = stateObjectProperties.Get()->GetShaderIdentifier(c_missShaderName);
    void* hitGroupShaderIdentifier = stateObjectProperties.Get()->GetShaderIdentifier(c_hitGroupName);
    UINT shaderIdentifierSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;

    // Ray gen shader table
    {
        struct RootArguments {
            RayGenConstantBuffer cb;
        } rootArguments;
        rootArguments.cb = rayGenCB;

        UINT numShaderRecords = 1;
        UINT shaderRecordSize = shaderIdentifierSize + sizeof(rootArguments);
        ShaderTable table(device.Get(), numShaderRecords, shaderRecordSize, L"RayGenShaderTable");
        table.push_back(ShaderRecord(rayGenShaderIdentifier, shaderIdentifierSize, &rootArguments, sizeof(rootArguments)));
        rayGenShaderTable = table.GetResource();
    }

    // Miss shader table
    {
        UINT numShaderRecords = 1;
        UINT shaderRecordSize = shaderIdentifierSize;
        ShaderTable table(device.Get(), numShaderRecords, shaderRecordSize, L"MissShaderTable");
        table.push_back(ShaderRecord(missShaderIdentifier, shaderIdentifierSize));
        missShaderTable = table.GetResource();
    }

    // Hit group shader table
    {
        UINT numShaderRecords = 1;
        UINT shaderRecordSize = shaderIdentifierSize;
        ShaderTable table(device.Get(), numShaderRecords, shaderRecordSize, L"HitGroupShaderTable");
        table.push_back(ShaderRecord(hitGroupShaderIdentifier, shaderIdentifierSize));
        hitGroupShaderTable = table.GetResource();
    }
}

void App3::CreateRaytracingOutputResource()
{
    auto& renderer = Application::Get().renderer;
    auto device = renderer.device;
    auto backbufferFormat = m_pWindow->GetBackBufferFormat();

    // Create the output resource. The dimensions and format should match the swap-chain.
    auto uavDesc = CD3DX12_RESOURCE_DESC::Tex2D(backbufferFormat, GetClientWidth(), GetClientHeight(), 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

    m_raytracingOutput.CreateUAV(renderer, uavDesc);
    
    // higher quality feedback
    uavDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    m_raytracingFeedback.CreateUAV(renderer, uavDesc);
}

void App3::ReleaseDeviceDependentResources()
{
    m_raytracingGlobalRootSignature.Reset();
    m_raytracingLocalRootSignature.Reset();

    Application::Get().renderer.dxrDevice.Reset();

    dxrStateObject.Reset();

    m_raytracingOutput.Reset();
    m_raytracingFeedback.Reset();

    meshA.Reset();
    meshB.Reset();

    topLevelAccelerationStructure.Reset();
}

void App3::CreateDeviceDependentResources()
{
    auto& renderer = Application::Get().renderer;

    // Create root signatures for the shaders.
    CreateRootSignatures();

    // Create a raytracing pipeline state object which defines the binding of shaders, state and resources to be used during raytracing.
    CreateRaytracingPipelineStateObject();

    // Allocate a heap for a large number of descriptors
    renderer.descriptorHeap.CreateDescriptorHeap(renderer, 1024 * 8, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
    renderer.descriptorHeap.maxSize = renderer.device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    bool ok;

    //meshA.load(renderer, "../../data/monkey.obj");
//    meshA.load(renderer, L"../../data/NewXYZ.obj");
//    ok = meshA.load(renderer, L"../../data/LShape.obj");
    ok = meshA.load(renderer, L"../../data/LShapeSmooth.obj");
    assert(ok);

//    ok = meshB.load(renderer, L"../../data/monkey.obj");
    ok = meshB.load(renderer, L"../../data/monkey2.obj");
    assert(ok);

    // set m_allIBandVB
    {
        UINT baseDescriptorIndexIB = 
            meshA.CreateSRVs(renderer, 0);
            meshB.CreateSRVs(renderer, 0);
        UINT baseDescriptorIndexVB =
            meshA.CreateSRVs(renderer, 1);
            meshB.CreateSRVs(renderer, 1);

        m_allIB = CD3DX12_GPU_DESCRIPTOR_HANDLE(
            renderer.descriptorHeap.descriptorHeap->GetGPUDescriptorHandleForHeapStart(),
            baseDescriptorIndexIB,
            renderer.descriptorHeap.maxSize);
        m_allVB = CD3DX12_GPU_DESCRIPTOR_HANDLE(
            renderer.descriptorHeap.descriptorHeap->GetGPUDescriptorHandleForHeapStart(),
            baseDescriptorIndexVB,
            renderer.descriptorHeap.maxSize);
    }


//meshB.load(renderer, "../../data/saucer.obj");
//    std::string inputfile = "../../data/monkey.obj";        // 1 shape
//    std::string inputfile = "../../data/NewXYZ.obj";          // many shapes
//    std::string inputfile = "../../data/LShape.obj";    // no clipping errors
//    std::string inputfile = "../../data/saucer.obj";
    //    std::string inputfile = "../../data/GroundPlane.obj";

    BuildAccelerationStructures();

    BuildShaderTables();

    CreateRaytracingOutputResource();

    // test
    m_texture.Load(renderer, "../../data/Textures/BlueNoise/LDR_RG01_0.png");
}
