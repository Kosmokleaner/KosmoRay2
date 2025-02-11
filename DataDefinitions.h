

STRUCT_BEGIN(Viewport)
	STRUCT_ENTRY(float, left)
	STRUCT_ENTRY(float, top)
	STRUCT_ENTRY(float, right)
	STRUCT_ENTRY(float, bottom)
STRUCT_END()

STRUCT_BEGIN(RayGenConstantBuffer)
	STRUCT_ENTRY(Viewport, viewport)
	STRUCT_ENTRY(Viewport, stencil)
STRUCT_END()

STRUCT_BEGIN(SceneConstantBuffer)
	STRUCT_ENTRY(float4x4, clipFromWorld)
	STRUCT_ENTRY(float4x4, worldFromClip)
	STRUCT_ENTRY(float4, cameraPosition)
	// .x:frac(time), y.:frac(time*0.1)
	STRUCT_ENTRY(float4, sceneParam0)
	STRUCT_ENTRY(uint, raytraceFlags)
	STRUCT_ENTRY(uint, FrameIndex)
	STRUCT_ENTRY(uint, wipeReservoir)
	STRUCT_ENTRY(uint, dummy[1])
STRUCT_END()

STRUCT_BEGIN(MaterialAttributes)
	STRUCT_ENTRY(float3, diffuseColor)
	STRUCT_ENTRY(float, specularPower)
	STRUCT_ENTRY(float3, specularColor)
	STRUCT_ENTRY(float, padding1)
	STRUCT_ENTRY(float3, emissiveColor)
	STRUCT_ENTRY(float, padding2)
STRUCT_END()

/*
ENUM_BEGIN(ETest2)
ENUM_ENTRY(eT_A)
ENUM_ENTRY(eT_B = 3)
ENUM_ENTRY(eT_C)
ENUM_ENTRY(eT_D = 5)
ENUM_END()
*/