#include "Camera.hpp"

namespace dx
{
    void Camera::Translate(DirectX::XMVECTOR translation, Space space) noexcept
    {
        using namespace DirectX;
        XMVECTOR finalTranslation;
        switch (space)
        {
        case Space::LocalSpace:
            finalTranslation = XMVector3Transform(translation, XMMatrixRotationQuaternion(GetRotation()));
            break;
        case Space::WorldSpace:
            finalTranslation = XMVectorAdd(GetTranslation(), translation);
            break;
        default:
            break;
        }
        XMStoreFloat4(&translation_, finalTranslation);
        translation_.w = 1.f;
    }

    void Camera::Rotate(DirectX::XMVECTOR rotation) noexcept
    {
        DirectX::XMStoreFloat4(&rotation_, DirectX::XMQuaternionMultiply(GetRotation(), rotation));
    }

    DirectX::XMVECTOR Camera::GetTranslation() const noexcept
    {
        return DirectX::XMLoadFloat4(&translation_);
    }

    DirectX::XMVECTOR Camera::GetRotation() const noexcept
    {
        return DirectX::XMLoadFloat4(&rotation_);
    }

    DirectX::XMMATRIX Camera::GetView() const noexcept
    {
        using namespace DirectX;
        const auto view = XMLoadFloat4x4(&viewMatrix_) /**
            XMMatrixTranspose(XMMatrixRotationQuaternion(GetRotation())) *
            XMMatrixTranslation(translation_.x, translation_.y, translation_.z)*/;
        return view;
    }

    DirectX::XMMATRIX Camera::GetProjection() const noexcept
    {
        using namespace DirectX;
        if (isProjectionDirty_)
        {
            isProjectionDirty_ = false;
            const auto result = XMMatrixPerspectiveFovLH(fov_, aspectRatio_, nearZ_, farZ_);
            XMStoreFloat4x4(&projectionMatrix_, result);
            return result;
        }
        else
        {
            return XMLoadFloat4x4(&projectionMatrix_);
        }
    }

    void Camera::SetProjection(float fov, float aspectRatio, float nearZ, float farZ) noexcept
    {
        isProjectionDirty_ = true;
        fov_ = fov;
        aspectRatio_ = aspectRatio;
        nearZ_ = nearZ;
        farZ_ = farZ;
    }

    void Camera::SetUvn(const DirectX::XMFLOAT3& eye, const DirectX::XMFLOAT3& target, const DirectX::XMFLOAT3& up) noexcept
    {
         DirectX::XMStoreFloat4x4(&viewMatrix_, 
             DirectX::XMMatrixLookAtLH(DirectX::XMLoadFloat3(&eye), 
                 DirectX::XMLoadFloat3(&target), 
                 DirectX::XMLoadFloat3(&up)));
    }

    void Camera::UpdateAspectRatio(float aspectRatio) noexcept
    {
        isProjectionDirty_ = true;
        aspectRatio_ = aspectRatio;
    }
}

