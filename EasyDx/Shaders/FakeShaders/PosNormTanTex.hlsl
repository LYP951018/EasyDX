#include "../Basic3D.hlsli"

float4 main(dx::Inputs::PosNormalTanTex input) : SV_POSITION { return input.Position; }