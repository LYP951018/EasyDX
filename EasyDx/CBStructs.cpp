#include "pch.hpp"
#include "CBStructs.hpp"
#include <d3d11.h>
#include "Material.hpp"
#include "Misc.hpp"

namespace dx::cb
{
    DirectX::XMFLOAT4 AsFloat4(const DirectX::XMFLOAT3& float3) noexcept
    {
        return { float3.x, float3.y, float3.z, 1.f };
    }
}

namespace dx::cb
{
    Light::Light(const dx::PointLight & point) noexcept
    {
        Type = LightType::PointLight;
        Position = MakePosition(point.Position);
        const auto& att = point.Attr;
        ConstantAttenuation = att.x;
        LinearAttenuation = att.y;
        QuadraticAttenuation = att.z;
        Color = point.Color;
        Range = point.Range;
        Enabled = point.Enabled;
    }

    Light::Light(const dx::DirectionalLight & directional) noexcept
    {
        Type = LightType::DirectionalLight;
        Direction = MakeDirection(directional.Direction);
        Color = directional.Color;
        Enabled = directional.Enabled;
    }

    Light::Light(const dx::SpotLight& spot) noexcept
    {
        Type = LightType::SpotLight;
        Position = MakePosition(spot.Position);
        Color = spot.Color;
        const auto& att = spot.Attr;
        ConstantAttenuation = att.x;
        LinearAttenuation = att.y;
        QuadraticAttenuation = att.z;
        Direction = MakeDirection(spot.Direction);
        SpotAngle = spot.SpotAngle;
        Range = spot.Range;
        Enabled = spot.Enabled;
    }

    Light::Light(const dx::Light& light) noexcept
    {
        struct Visitor
        {
            Light operator()(const PointLight& point) const noexcept
            {
                return Light{ point };
            }

            Light operator()(const DirectionalLight& directional) const noexcept
            {
                return Light{ directional };
            }

            Light operator()(const SpotLight& spot) const noexcept
            {
                return Light{ spot };
            }
        };
        *this = std::visit(Visitor{}, light);
    }

    Material::Material(const Smoothness& smoothness) noexcept
    {
        Ambient = smoothness.Amibient;
        Diffuse = smoothness.Diffuse;
        Specular = smoothness.Specular;
        Emissive = smoothness.Emissive;
        SpecularPower = smoothness.SpecularPower;
    }
}


