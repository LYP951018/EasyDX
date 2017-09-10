#pragma once

#include <DirectXMath.h>
#include <EasyDx/CBStructs.hpp>

namespace data
{
    struct alignas(16) CbPerObject
    {
        DirectX::XMMATRIX WorldViewProjection;
        DirectX::XMMATRIX World;
        DirectX::XMMATRIX InvTransposeWorld;
        dx::cb::data::Material MainMaterial;
    };

    struct alignas(16) CbPerFrame
    {
        DirectX::XMFLOAT4 EyePos;
        dx::cb::data::Light Dlight;
    };
}

using CbPerObject = dx::Cb<data::CbPerObject>;
using CbPerFrame = dx::Cb<data::CbPerFrame>;