#pragma once

#include "ComponentBase.hpp"
#include "AlignedAllocator.hpp"
#include <DirectXMath.h>

namespace dx
{
    class Transform
    {
      public:
        Transform();
        Transform(DirectX::XMVECTOR scale, DirectX::XMVECTOR rotation,
                  DirectX::XMVECTOR translation);

        DEFAULT_MOVE(Transform)

        DirectX::XMVECTOR Rotation() const;
        void SetRotation(DirectX::XMVECTOR rotation);

        const DirectX::XMFLOAT3& Position() const;
        void SetPosition(const DirectX::XMFLOAT3& position);

        const DirectX::XMFLOAT3& Scale() const;
        void SetScale(const DirectX::XMFLOAT3& scale);

        DirectX::XMMATRIX Matrix() const;

      private:
        struct alignas(16) Data
        {
            DirectX::XMVECTOR Rotation;
            DirectX::XMMATRIX Matrix;
        };

        mutable bool m_dirty;
        DirectX::XMFLOAT3 m_position;
        DirectX::XMFLOAT3 m_scale;
        aligned_unique_ptr<Data> m_data;
    };

    DEF_COMPONENT_WRAPPER_DEFAULT(Transform, GetTransform)

    DirectX::XMMATRIX MatrixFromTransform(dx::TransformComponent * transform);

}