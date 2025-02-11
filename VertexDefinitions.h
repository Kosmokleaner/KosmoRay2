

STRUCT_BEGIN(VFormatFull)
	STRUCT_ENTRY(float3, position)
	STRUCT_ENTRY(float3, normal)
	STRUCT_ENTRY(float2, uv)
	STRUCT_ENTRY(uint, materialId)		// inefficient to store it per vertex but simpler for now
STRUCT_END()
