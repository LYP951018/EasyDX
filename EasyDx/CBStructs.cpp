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

namespace dx
{
    IConstantBuffer::~IConstantBuffer()
    {
    }
}

namespace dx::cb::data
{
    void Light::FromPoint(const dx::PointLight & point) noexcept
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

    void Light::FromDirectional(const dx::DirectionalLight & directional) noexcept
    {
        Type = LightType::DirectionalLight;
        Direction = MakeDirection(directional.Direction);
        Color = directional.Color;
        Enabled = directional.Enabled;
    }

    void Light::FromSpot(const dx::SpotLight& spot) noexcept
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

    void Light::FromLight(const dx::Light & light) noexcept
    {
        struct Visitor
        {
            Light& Self;

            void operator()(const PointLight& point) const noexcept
            {
                Self.FromPoint(point);
            }

            void operator()(const DirectionalLight& directional) const noexcept
            {
                Self.FromDirectional(directional);
            }

            void operator()(const SpotLight& spot) const noexcept
            {
                Self.FromSpot(spot);
            }
        };
        std::visit(Visitor{ *this }, light);
    }

    void Material::FromSmoothness(const Smoothness & smoothness) noexcept
    {
        Ambient = smoothness.Amibient;
        Diffuse = smoothness.Diffuse;
        Specular = smoothness.Specular;
        Emissive = smoothness.Emissive;
        SpecularPower = smoothness.SpecularPower;
    }
}


