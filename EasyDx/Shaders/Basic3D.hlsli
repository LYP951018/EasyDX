#include "../ShaderDeclarations.hpp"

namespace dx
{
    namespace Inputs
    {
        struct Pos
        {
            float4 Position : POSITION;
        };

        struct PosColorTex
        {
            float4 Position : POSITION;
            float4 Color: COLOR;
            float2 TexCoord : TEXCOORD0;
        };

        struct PosNormal
        {
            float4 Position : POSITION;
            float4 Normal : NORMAL;
        };

        struct PosNormalTex
        {
            float4 Position : POSITION;
            float4 Normal : NORMAL;
            float2 TexCoord : TEXCOORD0;
        };

        struct PosNormalTanTex
        {
            float4 Position : POSITION;
            float4 Normal : NORMAL;
            float4 TangentU : TANGENT;
            float2 TexCoord : TEXCOORD0;
        };
    } // namespace Inputs

    namespace Outputs
    {
        struct Pos
        {
            float4 Position : SV_Position;
            float4 PositionWS : TEXCOORD0;
        };

        struct PosNormalTex
        {
            float4 Position : SV_Position;
            float4 PositionWS : TEXCOORD0;
            float3 NormalWS : TEXCOORD1;
            float2 TexCoord : TEXCOORD2;
        };
    } // namespace Output
} // namespace dx
