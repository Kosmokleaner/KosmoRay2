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

// only on NVidia
bool g_NVAPI_enabled = false;

const wchar_t* c_hitGroupName1 = L"MyHitGroupTri";
const wchar_t* c_hitGroupName2 = L"MyHitGroupAABB";

const wchar_t* c_raygenShaderName1 = L"BasePass";
const wchar_t* c_raygenShaderName2 = L"ShadingPass";
const wchar_t* c_closestHitShaderName = L"MyClosestHitShader";
const wchar_t* c_anyHitShaderName = L"MyAnyHitShader";
const wchar_t* c_missShaderName = L"MyMissShader";
const wchar_t* c_intersectShaderName = L"MyIntersectShader";

#define NUMBER_OF_OBJECT_IN_SCENE 5

namespace GlobalRootSignatureParams {
    enum Value {
        OutputViewSlot,             // DescriptorTable      UAV space0: u0(RenderTarget) space1: u0, u1 (NVidia)
        FeedbackSlot,               // DescriptorTable      UAV space0: u1(g_Feedback)
        ReservoirsSlot,             // DescriptorTable      UAV space0: u2(g_Reservoirs)
        GBufferASlot,               // DescriptorTable      UAV space0: u3(g_GBufferA)
        GBufferBSlot,               // DescriptorTable      UAV space0: u4(g_GBufferB)
        EmissiveSATValueSlot,       // DescriptorTable      UAV space0: u5(g_EmissiveSATValue)
        EmissiveSATIndexSlot,       // DescriptorTable      UAV space0: u6(g_EmissiveSATIndex)
        AccelerationStructureSlot,  // ShaderResourceView   SRV t0
        SceneConstant,              // ConstantBufferView   CBV b0
        IndexBuffer,                // DescriptorTable      SRV space101: t0: g_indices[IBIndex][]
        VertexBuffer,               // DescriptorTable      SRV space102: t0: g_vertices[VBIndex][]
        TextureSlot,                // DescriptorTable      SRV space103: t0: g_Texture[]
        SplatBuffer,                // DescriptorTable      SRV space104: t0: g_splats[BIndex][]
        MaterialBuffer,             // DescriptorTable      SRV space105: t0: g_materials[MatId]
        // -----
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
    rayGenCB.csViewport = { -1.0f, -1.0f, 1.0f, 1.0f };
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
        CD3DX12_ROOT_PARAMETER rootParameters[GlobalRootSignatureParams::Count] = {};

        {
            CD3DX12_DESCRIPTOR_RANGE UAVDescriptors[2] = {};
            UAVDescriptors[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0);       // space0: u0
            UAVDescriptors[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 2, 0, 1);       // space1: u0 and u1
            rootParameters[GlobalRootSignatureParams::OutputViewSlot].InitAsDescriptorTable(ARRAYSIZE(UAVDescriptors), UAVDescriptors);
        }

        {
            CD3DX12_DESCRIPTOR_RANGE UAVFeedback;
            UAVFeedback.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1, 0);             // space0: u1
            rootParameters[GlobalRootSignatureParams::FeedbackSlot].InitAsDescriptorTable(1, &UAVFeedback);
        }

        {
            CD3DX12_DESCRIPTOR_RANGE range;
            range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 2, 0);             // space0: u2
            rootParameters[GlobalRootSignatureParams::ReservoirsSlot].InitAsDescriptorTable(1, &range);
        }

        {
            CD3DX12_DESCRIPTOR_RANGE range;
            range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 3, 0);             // space0: u3
            rootParameters[GlobalRootSignatureParams::GBufferASlot].InitAsDescriptorTable(1, &range);
        }

        {
            CD3DX12_DESCRIPTOR_RANGE range;
            range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 4, 0);             // space0: u4
            rootParameters[GlobalRootSignatureParams::GBufferBSlot].InitAsDescriptorTable(1, &range);
        }

        {
            CD3DX12_DESCRIPTOR_RANGE range;
            range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 5, 0);             // space0: u5
            rootParameters[GlobalRootSignatureParams::EmissiveSATValueSlot].InitAsDescriptorTable(1, &range);
        }

        {
            CD3DX12_DESCRIPTOR_RANGE range;
            range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 6, 0);             // space0: u6
            rootParameters[GlobalRootSignatureParams::EmissiveSATIndexSlot].InitAsDescriptorTable(1, &range);
        }

        rootParameters[GlobalRootSignatureParams::AccelerationStructureSlot].InitAsShaderResourceView(0);   // 0 -> t0
        rootParameters[GlobalRootSignatureParams::SceneConstant].InitAsConstantBufferView(0);   // 0 -> b0

        {
            CD3DX12_DESCRIPTOR_RANGE SRVDescriptorIB[1] = {};
            SRVDescriptorIB[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0, 101);    // space101: t0: IndexBuffer, 2 SRV for 2 meshes
            rootParameters[GlobalRootSignatureParams::IndexBuffer].InitAsDescriptorTable(ARRAYSIZE(SRVDescriptorIB), SRVDescriptorIB);
        }

        {
            CD3DX12_DESCRIPTOR_RANGE SRVDescriptorVB[1] = {};
            SRVDescriptorVB[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0, 102);    // space102: t0: VertexBuffer, 2 SRV for 2 meshes
            rootParameters[GlobalRootSignatureParams::VertexBuffer].InitAsDescriptorTable(ARRAYSIZE(SRVDescriptorVB), SRVDescriptorVB);
        }

        {
            CD3DX12_DESCRIPTOR_RANGE SRVDescriptorTex[1] = {};
            SRVDescriptorTex[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 103);   // space103: t0: Texture
            rootParameters[GlobalRootSignatureParams::TextureSlot].InitAsDescriptorTable(ARRAYSIZE(SRVDescriptorTex), SRVDescriptorTex);
        }

        {
            CD3DX12_DESCRIPTOR_RANGE SRVDescriptorSplat[1] = {};
            SRVDescriptorSplat[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 104); // space104: t0: SplatBuffer
            rootParameters[GlobalRootSignatureParams::SplatBuffer].InitAsDescriptorTable(ARRAYSIZE(SRVDescriptorSplat), SRVDescriptorSplat);
        }

        {
            CD3DX12_DESCRIPTOR_RANGE SRVDescriptorMaterials[1] = {};
            SRVDescriptorMaterials[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 105); // space105: t0: Materials
            rootParameters[GlobalRootSignatureParams::MaterialBuffer].InitAsDescriptorTable(ARRAYSIZE(SRVDescriptorMaterials), SRVDescriptorMaterials);
        }

        {
            CD3DX12_ROOT_SIGNATURE_DESC globalRootSignatureDesc(ARRAYSIZE(rootParameters), rootParameters);
            SerializeAndCreateRaytracingRootSignature(globalRootSignatureDesc, &m_raytracingGlobalRootSignature);
        }
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
	m_sceneCB->resampling = resampling;
    m_sceneCB->emissiveSATSize = (uint32)m_emissiveSATValueData.size();
    m_sceneCB->emissiveSumArea = m_emissiveSumArea;
    m_sceneCB->emissiveSum = m_emissiveSum;

    static uint32 FrameIndex = 0; ++FrameIndex;
    m_sceneCB->FrameIndex = FrameIndex;

    {
        // clipFromWorld = clipFromEye * eyeFromWorld
        glm::mat4 clipFromWorld = m_ViewMatrix * m_ProjectionMatrix;
        m_sceneCB->cameraPosition = glm::vec4(camera.GetPos().x, camera.GetPos().y, camera.GetPos().z, 0.0f);
        m_sceneCB->clipFromWorld = glm::transpose(clipFromWorld);
        m_sceneCB->worldFromClip = glm::transpose(glm::inverse(clipFromWorld));
        glm::vec2 size(GetClientWidth(), GetClientHeight());
        m_sceneCB->frameBufferSize = glm::vec4((glm::vec2)size, 1.0f / (glm::vec2)size);
        m_sceneCB->mouseXY = glm::ivec4(mouseXY.x, mouseXY.y, 0, 0); 
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
	commandList->SetComputeRootDescriptorTable(GlobalRootSignatureParams::ReservoirsSlot, m_reservoirs.m_UAVGpuDescriptor);
    commandList->SetComputeRootDescriptorTable(GlobalRootSignatureParams::GBufferASlot, m_GBufferA.m_UAVGpuDescriptor);
    commandList->SetComputeRootDescriptorTable(GlobalRootSignatureParams::GBufferBSlot, m_GBufferB.m_UAVGpuDescriptor);
    commandList->SetComputeRootDescriptorTable(GlobalRootSignatureParams::EmissiveSATValueSlot, m_EmissiveSATValue.gpuDescriptorHandle);
	commandList->SetComputeRootDescriptorTable(GlobalRootSignatureParams::EmissiveSATIndexSlot, m_EmissiveSATIndex.gpuDescriptorHandle);
    commandList->SetComputeRootShaderResourceView(GlobalRootSignatureParams::AccelerationStructureSlot, topLevelAccelerationStructure->GetGPUVirtualAddress());

    // Set index and successive vertex buffer decriptor tables
//    commandList->SetComputeRootDescriptorTable(GlobalRootSignatureParams::IndexAndVertexBuffer, meshA.indexBuffer.gpuDescriptorHandle);
//    commandList->SetComputeRootDescriptorTable(GlobalRootSignatureParams::IndexAndVertexBuffer, meshB.indexBuffer.gpuDescriptorHandle);

    commandList->SetComputeRootDescriptorTable(GlobalRootSignatureParams::IndexBuffer, m_allIB);
    commandList->SetComputeRootDescriptorTable(GlobalRootSignatureParams::VertexBuffer, m_allVB);
    commandList->SetComputeRootDescriptorTable(GlobalRootSignatureParams::TextureSlot, m_texture.m_UAVGpuDescriptor);
	commandList->SetComputeRootDescriptorTable(GlobalRootSignatureParams::SplatBuffer, m_allSplats);
	commandList->SetComputeRootDescriptorTable(GlobalRootSignatureParams::MaterialBuffer, m_allMaterials);

    // hack
    ComPtr<ID3D12GraphicsCommandList4> m_dxrCommandList;
    ThrowIfFailed(commandList->QueryInterface(IID_PPV_ARGS(&m_dxrCommandList)), L"Couldn't get DirectX Raytracing interface for the command list.\n");

    // DispatchRays1
    {
        auto* commandList = m_dxrCommandList.Get();
        auto* stateObject = dxrStateObject.Get();

        // Since each shader table has only one shader record, the stride is same as the size.
        dispatchDesc.HitGroupTable.StartAddress = hitGroupShaderTable->GetGPUVirtualAddress();
        dispatchDesc.HitGroupTable.SizeInBytes = hitGroupShaderTable->GetDesc().Width;
        dispatchDesc.HitGroupTable.StrideInBytes = m_hitGroupShaderTableStrideInBytes;
        dispatchDesc.MissShaderTable.StartAddress = missShaderTable->GetGPUVirtualAddress();
        dispatchDesc.MissShaderTable.SizeInBytes = missShaderTable->GetDesc().Width;
        dispatchDesc.MissShaderTable.StrideInBytes = m_missShaderTableStrideInBytes;
        dispatchDesc.RayGenerationShaderRecord.StartAddress = rayGenShaderTable1->GetGPUVirtualAddress();
        dispatchDesc.RayGenerationShaderRecord.SizeInBytes = rayGenShaderTable1->GetDesc().Width;
        dispatchDesc.Width = GetClientWidth();
        dispatchDesc.Height = GetClientHeight();
        dispatchDesc.Depth = 1;
        commandList->SetPipelineState1(stateObject);
        commandList->DispatchRays(&dispatchDesc);
    }

    {
        // D3D12_RESOURCE_UAV_BARRIER 

        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
        barrier.UAV.pResource = m_raytracingOutput.m_resource.Get();
        commandList->ResourceBarrier(1, &barrier);
    }
    {
        // D3D12_RESOURCE_UAV_BARRIER 

        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
        barrier.UAV.pResource = m_GBufferA.m_resource.Get();
        commandList->ResourceBarrier(1, &barrier);
    }
    {
        // D3D12_RESOURCE_UAV_BARRIER 

        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
        barrier.UAV.pResource = m_GBufferB.m_resource.Get();
        commandList->ResourceBarrier(1, &barrier);
    }

    // DispatchRays2
    {
        auto* commandList = m_dxrCommandList.Get();
        auto* stateObject = dxrStateObject.Get();

        // Since each shader table has only one shader record, the stride is same as the size.
        dispatchDesc.HitGroupTable.StartAddress = hitGroupShaderTable->GetGPUVirtualAddress();
        dispatchDesc.HitGroupTable.SizeInBytes = hitGroupShaderTable->GetDesc().Width;
        dispatchDesc.HitGroupTable.StrideInBytes = m_hitGroupShaderTableStrideInBytes;
        dispatchDesc.MissShaderTable.StartAddress = missShaderTable->GetGPUVirtualAddress();
        dispatchDesc.MissShaderTable.SizeInBytes = missShaderTable->GetDesc().Width;
        dispatchDesc.MissShaderTable.StrideInBytes = m_missShaderTableStrideInBytes;
        dispatchDesc.RayGenerationShaderRecord.StartAddress = rayGenShaderTable2->GetGPUVirtualAddress();
        dispatchDesc.RayGenerationShaderRecord.SizeInBytes = rayGenShaderTable2->GetDesc().Width;
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
    // todo: combine with former barrier
	{
		// D3D12_RESOURCE_UAV_BARRIER 

		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
		barrier.UAV.pResource = m_reservoirs.m_resource.Get();
		commandList->ResourceBarrier(1, &barrier);
	}
	// todo: combine with former barrier
	{
		// D3D12_RESOURCE_UAV_BARRIER 

		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
		barrier.UAV.pResource = m_GBufferA.m_resource.Get();
		commandList->ResourceBarrier(1, &barrier);
    }
    // todo: combine with former barrier
    {
        // D3D12_RESOURCE_UAV_BARRIER 

        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
        barrier.UAV.pResource = m_GBufferB.m_resource.Get();
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
        // block the CPU thread until the specified fence value has been reached
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

        case KeyCode::Space:
            resampling = !resampling;
            log = true;
            break;
    }

    if(log)
    {
        char buffer[512];
        sprintf_s(buffer, "raytraceFlags: 0x%x resampling:%d\n", raytraceFlags, (int)resampling);
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
        rootSignatureAssociation->AddExport(c_raygenShaderName1);
        rootSignatureAssociation->AddExport(c_raygenShaderName2);
    }
}

float computeLumianceFromLinearRGB(glm::vec3 color)
{
    return dot(glm::vec3(0.2126f, 0.7152f, 0.0722f), color);
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
        lib->DefineExport(c_raygenShaderName1);
        lib->DefineExport(c_raygenShaderName2);
        lib->DefineExport(c_closestHitShaderName);
        lib->DefineExport(c_anyHitShaderName);
        lib->DefineExport(c_missShaderName);
		lib->DefineExport(c_intersectShaderName);
    }

    // Triangle hit group
    // A hit group specifies closest hit, any hit and intersection shaders to be executed when a ray intersects the geometry's triangle/AABB.
    // In this sample, we only use triangle geometry with a closest hit shader, so others are not set.
    {
        auto hitGroup = raytracingPipeline.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
		hitGroup->SetHitGroupExport(c_hitGroupName1); // must be unique
        hitGroup->SetClosestHitShaderImport(c_closestHitShaderName);
        hitGroup->SetAnyHitShaderImport(c_anyHitShaderName);
        hitGroup->SetHitGroupType(D3D12_HIT_GROUP_TYPE_TRIANGLES);
    }

    // AABB/procedural geometry hit group
	{
		auto hitGroup = raytracingPipeline.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
		hitGroup->SetHitGroupExport(c_hitGroupName2); // must be unique
		hitGroup->SetClosestHitShaderImport(c_closestHitShaderName);
		hitGroup->SetAnyHitShaderImport(c_anyHitShaderName);
        hitGroup->SetIntersectionShaderImport(c_intersectShaderName);
		hitGroup->SetHitGroupType(D3D12_HIT_GROUP_TYPE_PROCEDURAL_PRIMITIVE);
	}

    // Shader config
    // Defines the maximum sizes in bytes for the ray payload and attribute structure.
    auto shaderConfig = raytracingPipeline.CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
    // see struct RayPayload
	// color + emissive + normal + prim + instance + minT + minTfront
    UINT payloadSize = 4 * sizeof(float) + 4 * sizeof(float) + 3 * sizeof(float) + 4 + 4 + 4 + 4 + 4;

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
    splatA.BuildAccelerationStructures(Application::Get().renderer);

	auto& renderer = Application::Get().renderer;
    auto device = renderer.device;

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
    // Default heap is OK since the application doesn�t need CPU read/write access to them. 
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

    float sumArea = 0.0f;
    float sum = 0.0f;
    m_emissiveSATValueData.clear();
	m_emissiveSATIndexData.clear();

    uint32 instanceCount;
    {
        D3D12_RAYTRACING_INSTANCE_DESC instanceDesc[NUMBER_OF_OBJECT_IN_SCENE] = {};

        instanceCount = _countof(instanceDesc);

        D3D12_RAYTRACING_INSTANCE_DESC *dst = instanceDesc;

        // 0: emissive quad
        // /
        // 4: procedural
        for(UINT sceneObjectId = 0; sceneObjectId < instanceCount; ++sceneObjectId)
        {
            *dst = {};

            float size = 0.3f;

            // x: right, y:up (closer), z:behind
            if (sceneObjectId && sceneObjectId <= 3)
            {
                size = 1.5f;
                dst->Transform[sceneObjectId - 1][3] = (sceneObjectId == 2) ? 2.0f : 6.0f; 
            }
			
            if (sceneObjectId == 4)
				size = 1.0f;

            // hack off some objects and other stuff to make Cornell box
			{
				if (sceneObjectId == 0)
					size = 11.1f * 0.08f;
                if (sceneObjectId == 1 || sceneObjectId == 2 || sceneObjectId == 4)
                    size = 0.0f;
                dst->Transform[0][3] = 0.0f;
				dst->Transform[1][3] = 0.0f;
				dst->Transform[2][3] = 0.0f;

                if (sceneObjectId == 0)
					dst->Transform[1][3] = 2.95f;   // pretty close to the area light in the original Cornell box
            }

            dst->Transform[0][0] = dst->Transform[1][1] = dst->Transform[2][2] = size;
            dst->InstanceMask = 1;

            if (sceneObjectId == 4)
            {
                // AABB procedural

				dst->Transform[0][0] = dst->Transform[1][1] = dst->Transform[2][2] = size;
				dst->Transform[0][3] = 0.0f;
				dst->Transform[1][3] = 0.0f;
				dst->Transform[2][3] = 0.0f;

                dst->AccelerationStructure = splatA.bottomLevelAccelerationStructure->GetGPUVirtualAddress();
				// will go to HLSL InstanceID()
				dst->InstanceID = 0;
            }
            else
            {
                const uint32 meshIndex = (sceneObjectId == 0) ? 0 : 1;

                Mesh& ref = meshIndex ? meshB : meshA;

                dst->AccelerationStructure = ref.bottomLevelAccelerationStructure->GetGPUVirtualAddress();
                // will go to HLSL InstanceID()
                dst->InstanceID = meshIndex;
            }

			dst->InstanceContributionToHitGroupIndex = sceneObjectId;
//            dst->Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;
			dst->Flags = D3D12_RAYTRACING_INSTANCE_FLAG_FORCE_OPAQUE;

            // hack, we fix transform later
            if (sceneObjectId == 1)
            {
                dst->Transform[0][0] = dst->Transform[1][1] = dst->Transform[2][2] = 1.0f;
                dst->Transform[0][3] = 0.0f;
                dst->Transform[1][3] = 0.0f;
                dst->Transform[2][3] = 0.0f;
                dst->InstanceID = 1;
            }
            else
            {
                // don't show
				dst->Transform[0][0] = dst->Transform[1][1] = dst->Transform[2][2] = 0.0f;
				dst->Transform[0][3] = 0.0f;
				dst->Transform[1][3] = 0.0f;
				dst->Transform[2][3] = 0.0f;
				dst->InstanceID = 0;
            }

			// build emissiveSAT
            const Mesh* mesh = getMesh(dst->InstanceID);
            if(dst->InstanceID == 1) // hack, for now only meshB is emissive
            if(mesh)
			{
                uint32 triangleCount = mesh->getTriangleCount();
                assert(triangleCount);
                for (uint32 triangleId = 0; triangleId < triangleCount; ++triangleId)
                {
                    // all corner have the same value
                    uint32 materialid = mesh->MeshVertexData[mesh->MeshIndexData[triangleId * 3]].materialId;
                    glm::vec3 emissiveColor = mesh->materialAttributes[materialid].emissiveColor;
                    
                    if (emissiveColor == glm::vec3(0, 0, 0))
                        continue;

                    // https://stackoverflow.com/questions/596216/formula-to-determine-perceived-brightness-of-rgb-color
                    float emissiveAmount = computeLumianceFromLinearRGB(emissiveColor);

                    glm::vec3 p[3];

                    for (int e = 0; e < 3; ++e)
                    {
                        glm::vec3 osPos = mesh->getVertexAtIndex(triangleId * 3 + e); 

                        // todo  dst->Transform
                        glm::vec3 wsPos = osPos;
						p[e] = wsPos;
                    }

                    glm::vec3 u = p[2] - p[0];
                    glm::vec3 v = p[1] - p[0];

                    float triArea = length(cross(u, v)) / 2.0f;

                    sumArea += triArea;
                    sum += emissiveAmount * triArea;

                    m_emissiveSATValueData.push_back(sum);
                    m_emissiveSATIndexData.push_back(glm::uvec4(sceneObjectId, dst->InstanceID, triangleId, 0));
                }
            }

            ++dst;
        }

        AllocateUploadBuffer(device.Get(), instanceDesc, sizeof(instanceDesc), &instanceDescs, L"InstanceDescs");
    }

    m_emissiveSumArea = sumArea;

    // normalize emissiveSAT
    if(sum)
    {
        float inv = 1.0f / sum;

        for (auto& el : m_emissiveSATValueData)
            el *= inv;
    }
    else
    {
        // no emissive triangles
        m_emissiveSATValueData.push_back(0.0f);
    }

	AllocateUploadBuffer(device.Get(), m_emissiveSATValueData.data(), m_emissiveSATValueData.size() * sizeof(m_emissiveSATValueData[0]), &m_EmissiveSATValue.resource, L"EmissiveSATValue");
    renderer.CreateBufferSRV(&m_EmissiveSATValue, (uint32)m_emissiveSATValueData.size(), sizeof(m_emissiveSATValueData[0]));

    AllocateUploadBuffer(device.Get(), m_emissiveSATIndexData.data(), m_emissiveSATIndexData.size() * sizeof(m_emissiveSATIndexData[0]), &m_EmissiveSATIndex.resource, L"EmissiveSATIndex");
	renderer.CreateBufferSRV(&m_EmissiveSATIndex, (uint32)m_emissiveSATIndexData.size(), sizeof(m_emissiveSATIndexData[0]));

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
}

// This encapsulates all shader records - shaders and the arguments for their local root signatures.
void App3::BuildShaderTables()
{
    auto device = Application::Get().renderer.device;

    // Get shader identifiers.    
    ComPtr<ID3D12StateObjectProperties> stateObjectProperties;
    ThrowIfFailed(dxrStateObject.As(&stateObjectProperties));
    void* rayGenShaderIdentifier1 = stateObjectProperties.Get()->GetShaderIdentifier(c_raygenShaderName1);
    void* rayGenShaderIdentifier2 = stateObjectProperties.Get()->GetShaderIdentifier(c_raygenShaderName2);
    void* missShaderIdentifier = stateObjectProperties.Get()->GetShaderIdentifier(c_missShaderName);
    void* hitGroupShaderIdentifier1 = stateObjectProperties.Get()->GetShaderIdentifier(c_hitGroupName1);
	void* hitGroupShaderIdentifier2 = stateObjectProperties.Get()->GetShaderIdentifier(c_hitGroupName2);
    UINT shaderIdentifierSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;

	// A shader name look-up table for shader table debug print out.
	std::unordered_map<void*, std::wstring> shaderIdToStringMap;

    // todo: GetShaderIdentifier and this can be single line saving multiple lines of code
	shaderIdToStringMap[rayGenShaderIdentifier1] = c_raygenShaderName1;
    shaderIdToStringMap[rayGenShaderIdentifier2] = c_raygenShaderName2;
    shaderIdToStringMap[missShaderIdentifier] = c_missShaderName;
	shaderIdToStringMap[hitGroupShaderIdentifier1] = c_hitGroupName1;
	shaderIdToStringMap[hitGroupShaderIdentifier2] = c_hitGroupName2;

    // Ray gen shader table1
    {
        struct RootArguments {
            RayGenConstantBuffer cb;
        } rootArguments;
        rootArguments.cb = rayGenCB;

        UINT numShaderRecords = 1;
        UINT shaderRecordSize = shaderIdentifierSize + sizeof(rootArguments);
        ShaderTable table(device.Get(), numShaderRecords, shaderRecordSize, L"RayGenShaderTable1");
        table.push_back(ShaderRecord(rayGenShaderIdentifier1, shaderIdentifierSize, &rootArguments, sizeof(rootArguments)));
        table.DebugPrint(shaderIdToStringMap);
        rayGenShaderTable1 = table.GetResource();
    }
    // Ray gen shader table2
    {
        struct RootArguments {
            RayGenConstantBuffer cb;
        } rootArguments;
        rootArguments.cb = rayGenCB;

        UINT numShaderRecords = 1;
        UINT shaderRecordSize = shaderIdentifierSize + sizeof(rootArguments);
        ShaderTable table(device.Get(), numShaderRecords, shaderRecordSize, L"RayGenShaderTable2");
        table.push_back(ShaderRecord(rayGenShaderIdentifier2, shaderIdentifierSize, &rootArguments, sizeof(rootArguments)));
        table.DebugPrint(shaderIdToStringMap);
        rayGenShaderTable2 = table.GetResource();
    }

    // Miss shader table
    {
        UINT numShaderRecords = 1;
        UINT shaderRecordSize = shaderIdentifierSize;
        ShaderTable table(device.Get(), numShaderRecords, shaderRecordSize, L"MissShaderTable");
        table.push_back(ShaderRecord(missShaderIdentifier, shaderIdentifierSize));
		table.DebugPrint(shaderIdToStringMap);
		m_missShaderTableStrideInBytes = table.GetShaderRecordSize();
        missShaderTable = table.GetResource();
    }

    // Hit group shader table
    {
        // number of objects in the scene
        UINT numShaderRecords = NUMBER_OF_OBJECT_IN_SCENE;
        UINT shaderRecordSize = shaderIdentifierSize;
        ShaderTable table(device.Get(), numShaderRecords, shaderRecordSize, L"HitGroupShaderTable");
        table.push_back(ShaderRecord(hitGroupShaderIdentifier1, shaderIdentifierSize));
		table.push_back(ShaderRecord(hitGroupShaderIdentifier1, shaderIdentifierSize));
		table.push_back(ShaderRecord(hitGroupShaderIdentifier1, shaderIdentifierSize));
		table.push_back(ShaderRecord(hitGroupShaderIdentifier1, shaderIdentifierSize));
		table.push_back(ShaderRecord(hitGroupShaderIdentifier2, shaderIdentifierSize));
		table.DebugPrint(shaderIdToStringMap);
		m_hitGroupShaderTableStrideInBytes = table.GetShaderRecordSize();
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

    uavDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;    // todo: optimize
    m_GBufferA.CreateUAV(renderer, uavDesc);

    uavDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;   // float4(albedo,0)
    m_GBufferB.CreateUAV(renderer, uavDesc);

    uavDesc.Width *= 2; // 8 floats per pixel
    uavDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    m_reservoirs.CreateUAV(renderer, uavDesc);
}

void App3::ReleaseDeviceDependentResources()
{
    m_raytracingGlobalRootSignature.Reset();
    m_raytracingLocalRootSignature.Reset();

    Application::Get().renderer.dxrDevice.Reset();

    dxrStateObject.Reset();

    m_raytracingOutput.Reset();
    m_raytracingFeedback.Reset();
    m_reservoirs.Reset();

    meshA.Reset();
    meshB.Reset();
	splatA.Reset();

    topLevelAccelerationStructure.Reset();
}

const Mesh* App3::getMesh(uint32 meshInstanceId) const
{
	if (meshInstanceId == 0)
		return &meshA;
	if (meshInstanceId == 1)
		return &meshB;

	assert(0);
	return 0;
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
//    ok = meshA.load(renderer, L"../../data/LShapeSmooth.obj");
	ok = meshA.load(renderer, L"../../data/Quad.obj");
    assert(ok);

//    ok = meshB.load(renderer, L"../../data/monkey.obj");
//    ok = meshB.load(renderer, L"../../data/monkey2.obj");
	ok = meshB.load(renderer, L"../../data/CornellBox.obj");
//	ok = meshB.load(renderer, L"../../data/CornellBox2.obj");
    assert(ok);

    {
        uint32 count = 64;
        for (uint32 i = 0; i < count; ++i)
        {
            float f = i / (float)count * PI * 2;

            SplatData a;

//            a.Pos = glm::vec3(sinf(f) * 2, 0, cosf(f) * 2);
//            a.radius = 0.2f + 0.1f * sinf(f * 6.0f);
			a.position = glm::vec3(sinf(f) * 2, 0, cosf(f) * 2);
            a.radius = 0.2f + 0.1f * sinf(f * 6.0f);

            splatA.splatData.push_back(a);
        }
        splatA.CreateRenderMesh(renderer);
    }

    // set m_allIBandVB
    {
        UINT baseDescriptorIndexIB = 
            meshA.CreateSRVs(renderer, 0);
        meshB.CreateSRVs(renderer, 0);
		m_allIB = CD3DX12_GPU_DESCRIPTOR_HANDLE(
			renderer.descriptorHeap.descriptorHeap->GetGPUDescriptorHandleForHeapStart(),
			baseDescriptorIndexIB,
			renderer.descriptorHeap.maxSize);

        UINT baseDescriptorIndexVB =
            meshA.CreateSRVs(renderer, 1);
        meshB.CreateSRVs(renderer, 1);
		m_allVB = CD3DX12_GPU_DESCRIPTOR_HANDLE(
			renderer.descriptorHeap.descriptorHeap->GetGPUDescriptorHandleForHeapStart(),
			baseDescriptorIndexVB,
			renderer.descriptorHeap.maxSize);

	    UINT baseDescriptorMaterials =
			meshA.CreateSRVs(renderer, 2);
		meshB.CreateSRVs(renderer, 2);
		m_allMaterials = CD3DX12_GPU_DESCRIPTOR_HANDLE(
			renderer.descriptorHeap.descriptorHeap->GetGPUDescriptorHandleForHeapStart(),
			baseDescriptorMaterials,
			renderer.descriptorHeap.maxSize);

        UINT baseDescriptorIndexSplat =
            splatA.CreateSRVs(renderer);
		m_allSplats = CD3DX12_GPU_DESCRIPTOR_HANDLE(
			renderer.descriptorHeap.descriptorHeap->GetGPUDescriptorHandleForHeapStart(),
			baseDescriptorIndexSplat,
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

    // test
    m_texture.Load(renderer, "../../data/Textures/BlueNoise/LDR_RG01_0.png");
}
