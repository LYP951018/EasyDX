#include "../Basic3D.hlsli"

float4 main(dx::Inputs::PosNormal input) : SV_POSITION { return input.Position; }