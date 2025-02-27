

STRUCT_BEGIN(Viewport)
	STRUCT_ENTRY(float, left)
	STRUCT_ENTRY(float, top)
	STRUCT_ENTRY(float, right)
	STRUCT_ENTRY(float, bottom)
STRUCT_END()

// g_rayGenCB
STRUCT_BEGIN(RayGenConstantBuffer)
	// usually -1..1 -1..1
	STRUCT_ENTRY(Viewport, csViewport)
	STRUCT_ENTRY(Viewport, stencil)
STRUCT_END()

// g_sceneCB
STRUCT_BEGIN(SceneConstantBuffer)
	STRUCT_ENTRY(float4x4, clipFromWorld)
	STRUCT_ENTRY(float4x4, worldFromClip)
	STRUCT_ENTRY(float4, cameraPosition)
	// .x:frac(time), y.:frac(time*0.1)
	STRUCT_ENTRY(float4, sceneParam0)
	STRUCT_ENTRY(uint, raytraceFlags)
	STRUCT_ENTRY(uint, FrameIndex)
	STRUCT_ENTRY(uint, resampling)
	STRUCT_ENTRY(uint, emissiveSATSize)
    STRUCT_ENTRY(float, emissiveSumArea)
    STRUCT_ENTRY(float, emissiveSum)
    STRUCT_ENTRY(float, dummyB)
    STRUCT_ENTRY(float, dummyC)

//	STRUCT_ENTRY(uint, dummy)				// todo: dummy[1] will break next member
	// .xy:currrentXY, zw:unused
	STRUCT_ENTRY(int4, mouseXY)
	// .xy:size in pixels, .zw:1/size
	STRUCT_ENTRY(float4, frameBufferSize)
STRUCT_END()

STRUCT_BEGIN(MaterialAttributes)
	STRUCT_ENTRY(float3, diffuseColor)
	STRUCT_ENTRY(float, specularPower)
	STRUCT_ENTRY(float3, specularColor)
	STRUCT_ENTRY(float, padding1)
	STRUCT_ENTRY(float3, emissiveColor)
	STRUCT_ENTRY(float, padding2)
//todo HLSL need right padding 	CPP_STRUCT_ENTRY(std::string, debugName)	// only for C++
STRUCT_END()

/*
ENUM_BEGIN(ETest2)
ENUM_ENTRY(eT_A)
ENUM_ENTRY(eT_B = 3)
ENUM_ENTRY(eT_C)
ENUM_ENTRY(eT_D = 5)
ENUM_END()
*/