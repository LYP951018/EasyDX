#include "CBStructs.hpp"
#include "Material.hpp"
#include "Misc.hpp"

namespace dx::cb
{
    DirectX::XMFLOAT4 AsFloat4(const DirectX::XMFLOAT3& float3) noexcept
    {
        return { float3.x, float3.y, float3.z, 1.f };
    }

    Material Material::FromPlain(const dx::Smoothness& smoothness) noexcept
    {
        Material material;
        material.Ambient = smoothness.Amibient;
        material.Diffuse = smoothness.Diffuse;
        material.Specular = smoothness.Specular;
        material.Emissive = smoothness.Emissive;
        material.SpecularPower = smoothness.SpecularPower;
        return material;
    }

    struct LightVisitor
    {
        Light operator()(const PointLight& point) const noexcept
        {
            return Light::FromPoint(point);
        }

        Light operator()(const DirectionalLight& directional) const noexcept
        {
            return Light::FromDirectional(directional);
        }

        Light operator()(const SpotLight& spot) const noexcept
        {
            return Light::FromSpot(spot);
        }
    };

    Light Light::FromPlain(const dx::Light& light) noexcept
    {
        return std::visit(LightVisitor{}, light);
    }

    Light Light::FromPoint(const dx::PointLight& pointLight) noexcept
    {
        Light light;
        light.Type = LightType::PointLight;
        light.Position = MakePosition(pointLight.Position);
        const auto& att = pointLight.Attr;
        light.ConstantAttenuation = att.x;
        light.LinearAttenuation = att.y;
        light.QuadraticAttenuation = att.z;
        light.Color = pointLight.Color;
        light.Range = pointLight.Range;
        light.Enabled = pointLight.Enabled;
        return light;
    }

    Light Light::FromDirectional(const dx::DirectionalLight& directionalLight) noexcept
    {
        Light light;
        light.Type = LightType::DirectionalLight;
        light.Direction = MakeDirection(directionalLight.Direction);
        light.Color = directionalLight.Color;
        light.Enabled = directionalLight.Enabled;
        return light;
    }

    Light Light::FromSpot(const dx::SpotLight& spotLight) noexcept
    {
        Light light;
        light.Type = LightType::SpotLight;
        light.Position = MakePosition(spotLight.Position);
        light.Color = spotLight.Color;
        const auto& att = spotLight.Attr;
        light.ConstantAttenuation = att.x;
        light.LinearAttenuation = att.y;
        light.QuadraticAttenuation = att.z;
        light.Direction = MakeDirection(spotLight.Direction);
        light.SpotAngle = spotLight.SpotAngle;
        light.Range = spotLight.Range;
        light.Enabled = spotLight.Enabled;
        return light;
    }
}
