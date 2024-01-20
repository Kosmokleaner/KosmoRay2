KosmoRay2     by Martin Mittring    11/17/2022

Multiple experiments in different Apps:
  comment one of the lines starting with   std::shared_ptr<App2> demo = std::make_shared<App2>(L"KosmoRay2 App2 DX12 Rasterization", 1280, 720);
  "App2": rasterization
  "App3": ray tracing, Constructive Solid Geometry (CSG) Intersection of sphere and 3d mesh object

Code style (not yet fully executed):
* no hungarian notation, todo: remove m_ ?
* Camel case for functions/methods/member variables
* No "." at end of comments that are not sentences
* 

todo:
* support multiple vertex buffer for ray tracing
* raytrace scene object
* shader class
* with Mock enabled RenderDoc crashes
* move TransitionResource, ClearRTV, ClearDepth, UpdateBufferResource, ResizeDepthBuffer out of App
* move NVAPI out of App
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
* DearGPU, various helper code, e.g. OBJ loader, WTFPL http://www.wtfpl.net by MartinMittring / Epic Games
  * Tom Forsyth vertex cache optimization code
  * TangentSpaceMeshCalculation by MartinMittring / Crytek
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

What is D3D12_CPU_DESCRIPTOR_HANDLE

DXR spec https://microsoft.github.io/DirectX-Specs/d3d/Raytracing.html



Resource Binding in DirectX 12 (pt.1)
https://www.youtube.com/watch?v=Uwhhdktaofg
https://www.youtube.com/watch?v=Wbnw87tYqVg&t=0s
* DescriptorHeap stores Descriptors, can be shader visible (CPU write, GPU read) or not shader visible (to prepare to upload to shader visible, not needed)
  need to organize by type, exception: CBV,SRV,UAV can be in one
  IBV,VBV,SOV,RTV,DSV are CPU / non shader visible, hardware does not use
  after creation get handle to start, GetCPUHandleForHeapStart(), GetGPUHandleForHeapStart()
* GetDescriptoHandleIncrementSize() 
  can use wrapper class: CD3DX12_CPU_DESCRIPTOR_HANDLE .. 
* Descriptor 32-64 bytes for CBV,SRV,UAV
* DescriptorHandle device-unique address 64bit but descriptors are 32bit as they are relative to start
* RootSignature user defined order of types structured, can be defined in HLSL (e.g. define samplers in HLSL)
  root parameter types: 
   descriptor table (indirection to descriptors) 1 DWORD (offset into descriptor heap), 2 indirections
   root descriptor (inline descriptor) 2 DWORD (virtual address), 1 indirection
   root constants (inline constants) 1 DWORD, 0 indirection
   static sampler (baked into shader)
   
* PlacedResource
* Root arguments are limited to 64 DWORDs

todo: https://www.youtube.com/watch?v=Wbnw87tYqVg&t=0s ~32 min



Current app resource management explained:     (deconstructing app would be useful)

