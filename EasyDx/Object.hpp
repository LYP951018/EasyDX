#pragma once

#include <DirectXMath.h>
#include "Material.hpp"

namespace dx
{
    //TODO: What's the Pipeline concept?
    //TODO: ECS?
    template<typename PipelineT>
    struct Object
    {
        Object(PipelineT pipeline, const Smoothness& smoothness, DirectX::XMMATRIX transform)
            : Pipeline_{pipeline},
            Material{smoothness}
        {
            DirectX::XMStoreFloat4x4(&Transform, transform);
        }

        //TODO: What's this
        DirectX::XMMATRIX GetWorld() const noexcept
        {
            return DirectX::XMLoadFloat4x4(&Transform);
        }

        PipelineT Pipeline_;
        Smoothness Material;
        DirectX::XMFLOAT4X4 Transform;
    };
}