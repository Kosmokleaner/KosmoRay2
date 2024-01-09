KosmoRay2     by Martin Mittring    11/17/2022

Multiple experiments in different Apps:
  comment one of the lines starting with   std::shared_ptr<App2> demo = std::make_shared<App2>(L"KosmoRay2 App2 DX12 Rasterization", 1280, 720);
  "App2": rasterization
  "App3": ray tracing, Constructive Solid Geometry (CSG) Intersection of sphere and 3d mesh object


todo:
* App code should be as simple as possible
* remove "hack"
* remove C++ exception handling
* implement more classic renderer stuff (Resource management / barriers, draw call rendering, GI, AO, TAA, Tonemapper, Bloom, ReSTIR)
* renderer in it's own process => update code without quit
* multi draw indirect
* gaussian splatting ray tracer
* map all Dx12 features by reimplementing API wrapper
* implement handle based or shared_ptr resource management

todo low prio:
* Mock12 should be enabled by define (search for "new Mock12Device2")
* fix: window resize crash
* fps should not go to OutputDebugString
* fix hack, to query m_dxrCommandList each time
* support uvs and normals from obj load
* obj load vertex cache optimize


ThirdParty / Copyright:
* LearningDirectX12 https://github.com/jpvanoosten/LearningDirectX12, MIT license
* tiny_obj_loader, to load Alias Wavefront .obj files, MIT license
* DearGPU, various helper code, WTFPL http://www.wtfpl.net
* Cellular noise ("Worley noise") in 3D in GLSL. Stefan Gustavson MIT license. https://github.com/stegu/webgl-noise



==========================================

References:


Adding Ray tracing:
https://nvpro-samples.github.io/vk_raytracing_tutorial_KHR/
https://developer.nvidia.com/rtx/raytracing/dxr/dx12-raytracing-tutorial-part-1


Game Engine Programming 035.1 - DirectX 12 pipelines and the root signature | C++ Game Engine
https://www.youtube.com/watch?v=1RkBThKwnQE


shading knowledge
https://blog.selfshadow.com/publications/s2015-shading-course/hoffman/s2015_pbs_physics_math_slides.pdf

=============================

What is a root signature ?
What is D3D12_CPU_DESCRIPTOR_HANDLE

DXR spec https://microsoft.github.io/DirectX-Specs/d3d/Raytracing.html


D3D12 ERROR: ID3D12CommandQueue::ExecuteCommandLists: Using ResourceBarrier on Command List (0x000001BAD2D28F30:'Unnamed ID3D12GraphicsCommandList Object'):
 Before state (0x4: D3D12_RESOURCE_STATE_RENDER_TARGET) of resource (0x000001BAD2D024F0:'Unnamed ID3D12Resource Object') (subresource: 0) specified by
transition barrier does not match with the state (0x0: D3D12_RESOURCE_STATE_[COMMON|PRESENT]) specified in the previous call to ResourceBarrier [
 RESOURCE_MANIPULATION ERROR #527: RESOURCE_BARRIER_BEFORE_AFTER_MISMATCH]

D3D12: **BREAK** enabled for the previous message, which was: [ ERROR RESOURCE_MANIPULATION #527: RESOURCE_BARRIER_BEFORE_AFTER_MISMATCH ]
Exception thrown at 0x00007FFDFC3ECD29 (KernelBase.dll) in KosmoRay2.exe: 0x0000087A (parameters: 0x0000000000000001, 0x00000065938FA040, 0x00000065938FBE10).
Unhandled exception at 0x00007FFDFC3ECD29 (KernelBase.dll) in KosmoRay2.exe: 0xC000041D: An unhandled exception was encountered during a user callback.




