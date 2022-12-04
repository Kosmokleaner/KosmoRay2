KosmoRay2     by Martin Mittring    11/17/2022

todo:
* 

todo low prio:
* fix: window resize crash
* fix hack, to query m_dxrCommandList each time
* support uvs and normals from obj load
* obj load vertex cache optimize


ThirdParty / Copyright:
* LearningDirectX12 https://github.com/jpvanoosten/LearningDirectX12, MIT license
* tiny_obj_loader, to load Alias Wavefront .obj files, MIT license
* DearGPU, various helper code, WTFPL http://www.wtfpl.net


 ==========================================


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




