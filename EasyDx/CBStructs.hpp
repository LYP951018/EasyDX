#pragma once

#include "Light.hpp"
#include <DirectXMath.h>
#include <cstdint>

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
            bool Enabled;
            float Range;
            std::uint32_t Padding;

            static Light FromPlain(const dx::Light& light) noexcept;
            static Light FromPoint(const dx::PointLight& pointLight) noexcept;
            static Light FromDirectional(const dx::DirectionalLight& directionalLight) noexcept;
            static Light FromSpot(const dx::SpotLight& spotLight) noexcept;
        };

        struct alignas(16) Material
        {
            DirectX::XMFLOAT4 Ambient;
            DirectX::XMFLOAT4 Diffuse;
            DirectX::XMFLOAT4 Specular;
            DirectX::XMFLOAT4 Emissive;

            float SpecularPower;
            DirectX::XMFLOAT3 Padding;

            static Material FromPlain(const dx::Smoothness& smoothness) noexcept;
        };
    }
}