#pragma once

#include <DirectXMath.h>

namespace dx
{
    struct Smoothness
    {
        DirectX::XMFLOAT4 Amibient, Diffuse, Specular, Emissive;
        float SpecularPower;
    };
}