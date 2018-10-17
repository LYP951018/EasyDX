#include "../Basic3D.hlsli"

float4 main(dx::Inputs::PosNormalTex input) : SV_POSITION { return input.Position; }