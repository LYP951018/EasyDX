namespace dx
{
    struct InstancedVertexInput
    {
        float4 Position : POSITION;
        float4 Normal : NORMAL;
        float2 TexCoord : TEXCOORD;

        matrix WorldMatrix : WORLDMATRIX;
        matrix InvTransWorldMatrix : INVTRANSWORLDMATRIX;
    };

    struct InstancedVertexOutput
    {
        float4 PositionWS : TEXCOORD1;
        float3 NormalWS : TEXCOORD2;
        float2 TexCoord : TEXCOORD0;
        float4 Position : SV_Position;
    };

    struct VertexInput
    {
        float4 Position : POSITION;
        float4 Normal : NORMAL;
        float4 TangentU : TANGENT;
        float2 TexCoord : TEXCOORD;
    };

    struct VertexOutput
    {
        float4 Position : SV_Position;
        float4 PositionWS : TEXCOORD0;
        float3 NormalWS : TEXCOORD1;
        float2 TexCoord : TEXCOORD2;
    };
}


