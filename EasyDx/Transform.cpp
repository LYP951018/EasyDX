#include "pch.hpp"
#include "Transform.hpp"

namespace dx
{
    Transform::Transform()
        : m_dirty{true}, m_position{}, m_scale{1.0f, 1.0f, 1.0f}, m_data{aligned_unique<Data>()}
    {
        m_data->Rotation = DirectX::XMQuaternionIdentity();
    }

    Transform::Transform(DirectX::XMVECTOR scale, DirectX::XMVECTOR rotation,
                         DirectX::XMVECTOR translation)
        : m_dirty{true}, m_data{aligned_unique<Data>()}
    {
        DirectX::XMStoreFloat3(&m_scale, scale);
        DirectX::XMStoreFloat3(&m_position, translation);
        m_data->Rotation = rotation;
    }

    DirectX::XMVECTOR Transform::Rotation() const { return m_data->Rotation; }

    void Transform::SetRotation(DirectX::XMVECTOR rotation)
    {
        m_data->Rotation = rotation;
        m_dirty = true;
    }

    const DirectX::XMFLOAT3& Transform::Position() const { return m_position; }

    void Transform::SetPosition(const DirectX::XMFLOAT3& position)
    {
        m_position = position;
        m_dirty = true;
    }

    const DirectX::XMFLOAT3& Transform::Scale() const { return m_scale; }

    void Transform::SetScale(const DirectX::XMFLOAT3& scale)
    {
        m_scale = scale;
        m_dirty = true;
    }

    DirectX::XMMATRIX Transform::Matrix() const
    {
        using namespace DirectX;
        if (m_dirty)
        {
            const auto position = XMLoadFloat3(&m_position);
            const auto scale = XMLoadFloat3(&m_scale);
            m_data->Matrix =
                XMMatrixAffineTransformation(scale, XMVectorZero(), m_data->Rotation, position);
        }
        return m_data->Matrix;
    }

    DirectX::XMMATRIX MatrixFromTransform(dx::TransformComponent * transform)
    {
        return transform == nullptr ? DirectX::XMMatrixIdentity() : transform->GetTransform().Matrix();
    }
} // namespace dx