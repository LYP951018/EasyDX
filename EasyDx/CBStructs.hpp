#pragma once

#include "Light.hpp"
#include <DirectXMath.h>

namespace dx
{
    struct Smoothness;

    namespace cb
    {
        struct alignas(16) Light
        {
            DirectX::XMFLOAT4 Position;
            DirectX::XMFLOAT4 Direction;
            DirectX::XMFLOAT4 Color;

            float SpotAngle;
            float ConstantAttenuation;
            float LinearAttenuation;
            float QuadraticAttenuation;

            LightType Type;
            std::int32_t Enabled;
            float Range;
            std::uint32_t Padding;

            Light(const PointLight& point) noexcept;
            Light(const DirectionalLight& directional) noexcept;
            Light(const SpotLight& spot) noexcept;
            Light(const dx::Light& light) noexcept;
            Light() = default;
        };

        struct alignas(16) Material
        {
            DirectX::XMFLOAT4 Ambient;
            DirectX::XMFLOAT4 Diffuse;
            DirectX::XMFLOAT4 Specular;
            DirectX::XMFLOAT4 Emissive;

            float SpecularPower;
            std::int32_t UseTexture;
            DirectX::XMFLOAT2 Padding;

            Material(const Smoothness& smoothness, bool useTexture) noexcept;
        };
    }
}