#include "../Basic3D.hlsli"

float4 main(dx::Inputs::Pos input) : SV_POSITION { return input.Position; }