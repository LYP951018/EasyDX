#pragma once

#include "Component.hpp"
#include <DirectXMath.h>

namespace dx
{
    struct Smoothness : IComponent
    {
        Smoothness(const DirectX::XMFLOAT4& ambient,
            const DirectX::XMFLOAT4& diffuse, const DirectX::XMFLOAT4& specular, const DirectX::XMFLOAT4& emissive,
            float power)
            :Amibient{ambient}, Diffuse{diffuse}, Specular{specular}, Emissive{emissive},
            SpecularPower{power}
        {}

        Smoothness() = default;

        std::uint32_t GetId() const override
        {
            return ComponentId::kSmoothness;
        }

        static std::uint32_t GetStaticId()
        {
            return ComponentId::kSmoothness;
        }

        DirectX::XMFLOAT4 Amibient, Diffuse, Specular, Emissive;
        float SpecularPower;
    };
}