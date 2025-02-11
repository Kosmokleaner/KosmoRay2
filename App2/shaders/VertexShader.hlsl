struct ModelViewProjection
{
    matrix MVP;
};

// model->clip aka clipFromModel
ConstantBuffer<ModelViewProjection> ModelViewProjectionCB : register(b0);

struct VFormatFull
{
    float3 Position : POSITION;
    float3 Normal : TEXCOORD2;
    float2 UV    : TEXCOORD3;
    uint materialId : TEXCOORD4;
};

struct VertexShaderOutput
{
    float4 Position : SV_Position;
    float3 Normal : TEXCOORD2;
    float2 UV    : TEXCOORD3;
};

VertexShaderOutput main(VFormatFull IN)
{
    VertexShaderOutput OUT;

    // model->clip aka clipFromModel
    OUT.Position = mul(ModelViewProjectionCB.MVP, float4(IN.Position, 1.0f));
    OUT.Normal = IN.Normal;
    OUT.UV = IN.UV;

    return OUT;
}