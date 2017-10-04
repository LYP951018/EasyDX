#pragma once

#include <DirectXMath.h>
#include <variant>

namespace dx
{
    enum class LightType : std::uint32_t
    {
        DirectionalLight = 0,
        PointLight,
        SpotLight,
    };

    struct PointLight
    {
        DirectX::XMFLOAT3 Position;
        DirectX::XMFLOAT4 Color;
        DirectX::XMFLOAT3 Attr;
        float Range;
        bool Enabled;
    };

    struct DirectionalLight
    {
        DirectX::XMFLOAT4 Color;
        DirectX::XMFLOAT3 Direction;
        bool Enabled;
    };

    struct SpotLight
    {
        DirectX::XMFLOAT3 Position;
        float SpotAngle;
        DirectX::XMFLOAT3 Direction;
        float Range;
        DirectX::XMFLOAT4 Color;
        DirectX::XMFLOAT3 Attr;
        bool Enabled;
    };

    using Light = std::variant<PointLight, DirectionalLight, SpotLight>;
}