struct PixelShaderInput
{
    float4 Position : SV_Position;
    float3 Normal : TEXCOORD2;
    float2 UV    : TEXCOORD3;
};

float4 main( PixelShaderInput IN ) : SV_Target
{
    return float4(IN.UV, 0, 0);
}