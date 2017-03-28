#include "Camera.hpp"
#include "Buffers.hpp"
#include <d3d11.h>
#include <Windows.h>

namespace dx
{
    Camera::Camera() noexcept
        : isProjectionDirty_{true},
        isViewDirty_{true},
        rotation_{0.f, 0.f, 0.f, 1.f},
        translation_{0.f, 0.f, 0.f, 1.f}
    {
    }

    void Camera::Translate(float x, float y, float z, Space space) noexcept
    {
        using namespace DirectX;
        XMVECTOR finalTranslation;
        const DirectX::XMFLOAT4 translation = { x, y, z, 1.f };
        const auto translationVec = DirectX::XMLoadFloat4(&translation);
        switch (space)
        {
        case Space::LocalSpace:
            finalTranslation = XMVector3Transform(translationVec, XMMatrixRotationQuaternion(LoadRotation()));
            break;
        case Space::WorldSpace:
            finalTranslation = XMVectorAdd(LoadTranslation(), translationVec);
            break;
        default:
            std::terminate();
            break;
        }
        XMStoreFloat4(&translation_, finalTranslation);
        translation_.w = 1.f;
        isViewDirty_ = true;
    }

    void Camera::Rotate(DirectX::XMVECTOR rotation) noexcept
    {
        DirectX::XMStoreFloat4(&rotation_, DirectX::XMQuaternionMultiply(LoadRotation(), rotation));
        isViewDirty_ = true;
    }

    DirectX::XMVECTOR Camera::LoadTranslation() const noexcept
    {
        return DirectX::XMLoadFloat4(&translation_);
    }

    DirectX::XMVECTOR Camera::LoadRotation() const noexcept
    {
        return DirectX::XMLoadFloat4(&rotation_);
    }

    void Camera::SetProjection(float fov, float aspectRatio, float nearZ, float farZ) noexcept
    {
        isProjectionDirty_ = true;
        fov_ = fov;
        aspectRatio_ = aspectRatio;
        nearZ_ = nearZ;
        farZ_ = farZ;
    }

    void Camera::SetLookAt(const DirectX::XMFLOAT3& eye, const DirectX::XMFLOAT3& target, const DirectX::XMFLOAT3& up) noexcept
    {
        const auto view = DirectX::XMMatrixLookAtLH(DirectX::XMLoadFloat3(&eye),
            DirectX::XMLoadFloat3(&target),
            DirectX::XMLoadFloat3(&up));
        translation_ = { eye.x, eye.y, eye.z, 1.f };
        DirectX::XMStoreFloat4(&rotation_, DirectX::XMQuaternionRotationMatrix(XMMatrixTranspose(view)));
        isViewDirty_ = true;
    }

    void Camera::UpdateAspectRatio(float aspectRatio) noexcept
    {
        isProjectionDirty_ = true;
        aspectRatio_ = aspectRatio;
    }

    bool Camera::HasChanged() const noexcept
    {
        return isViewDirty_ || isProjectionDirty_;
    }

    DirectX::XMMATRIX Camera::GetView() const noexcept
    {
        if (isViewDirty_)
        {
            auto rotation = XMMatrixTranspose(DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&rotation_)));
            auto translation = DirectX::XMMatrixTranslation(-translation_.x, -translation_.y, -translation_.z);
            const auto result = translation * rotation;
            DirectX::XMStoreFloat4x4(&view_, result);
            isViewDirty_ = false;
            return result;
        }
        else
        {
            return DirectX::XMLoadFloat4x4(&view_);
        }
    }

    DirectX::XMMATRIX Camera::GetProjection() const noexcept
    {
        if (isProjectionDirty_)
        {
            const auto result = DirectX::XMMatrixPerspectiveFovLH(fov_, aspectRatio_, nearZ_, farZ_);
            DirectX::XMStoreFloat4x4(&projection_, result);
            isProjectionDirty_ = false;
            return result;
        }
        else
        {
            return DirectX::XMLoadFloat4x4(&projection_);
        }
    }
}

