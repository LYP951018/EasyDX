struct PSInput
{
    float4 WorldPos : TEXCOORD0;
    float4 WorldNormal : TEXCOORD1;
    float4 ProjectedPos : SV_Position;
    float2 TexCoord : TEXCOORD2;
};