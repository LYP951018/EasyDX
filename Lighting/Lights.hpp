#pragma once

#include <DirectXMath.h>


struct alignas(16) DirectionalLight
{
    DirectX::XMFLOAT4 Ambient, Diffuse, Specular;
    DirectX::XMFLOAT3 Direction;
};

struct alignas(16) PointLight
{
    DirectX::XMFLOAT4 Ambient, Diffuse, Specular;
    DirectX::XMFLOAT3 Position;
    float Range;
    DirectX::XMFLOAT3 Attenuation;
};

struct alignas(16) SpotLight
{
    DirectX::XMFLOAT4 Ambient, Diffuse, Specular;
    DirectX::XMFLOAT3 Position;
    float Range;
    DirectX::XMFLOAT3 Direction;
    float Spot; //The exponent used in the spotlight calculation to control the spotlight cone.
    DirectX::XMFLOAT3 Attenuation; //control how light intensity falls off with distance.
};