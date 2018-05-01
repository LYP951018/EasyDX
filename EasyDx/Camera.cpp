#include "pch.hpp"
#include "Camera.hpp"
#include "Resources/Buffers.hpp"
#include <d3d11.h>
#include <Windows.h>

namespace dx
{
    namespace Internal
    {
        struct alignas(16) CameraData
        {
            DirectX::XMMATRIX View;
            DirectX::XMMATRIX Projection;
        };
    }

    Camera::Camera()
        : isProjectionDirty_{true},
        isViewDirty_{true},
        rotation_{0.f, 0.f, 0.f, 1.f},
        position_{0.f, 0.f, 0.f, 1.f},
        data_{aligned_unique<Internal::CameraData>()},
        viewport_{std::make_unique<D3D11_VIEWPORT>()}
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
            finalTranslation = XMVectorAdd(LoadTranslation(), XMVector3Transform(translationVec, XMMatrixRotationQuaternion(LoadRotation())));
            break;
        case Space::WorldSpace:
            finalTranslation = XMVectorAdd(LoadTranslation(), translationVec);
            break;
        default:
            std::terminate();
            break;
        }
        XMStoreFloat4(&position_, finalTranslation);
        position_.w = 1.f;
        isViewDirty_ = true;
    }

    void Camera::Pitch(float angle)
    {
        Rotate(DirectX::XMQuaternionRotationNormal(DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), angle));
    }

    void Camera::Yaw(float angle)
    {
        Rotate(DirectX::XMQuaternionRotationNormal(DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), angle));
    }

    void Camera::RotateY(float angle)
    {
        const auto rotation = DirectX::XMQuaternionRotationNormal(DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), angle);
        Rotate(rotation);
        DirectX::XMStoreFloat4(&position_, DirectX::XMQuaternionMultiply(LoadTranslation(), rotation));
    }

    void Camera::RotateX(float angle)
    {
        const auto rotation = DirectX::XMQuaternionRotationNormal(DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), angle);
        Rotate(rotation);
        DirectX::XMStoreFloat4(&position_, DirectX::XMQuaternionMultiply(LoadTranslation(), rotation));
    }

    void Camera::Rotate(DirectX::XMVECTOR rotation) noexcept
    {
        DirectX::XMStoreFloat4(&rotation_, DirectX::XMQuaternionMultiply(LoadRotation(), rotation));
        isViewDirty_ = true;
    }

    DirectX::XMVECTOR Camera::LoadTranslation() const noexcept
    {
        return DirectX::XMLoadFloat4(&position_);
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
        position_ = { eye.x, eye.y, eye.z, 1.f };
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
        auto& view = data_->View;
        if (isViewDirty_)
        {
            auto rotation = XMMatrixTranspose(DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&rotation_)));
            auto translation = DirectX::XMMatrixTranslation(-position_.x, -position_.y, -position_.z);
            view = translation * rotation;
            isViewDirty_ = false;
        }
        return view;
    }

    DirectX::XMMATRIX Camera::GetProjection() const noexcept
    {
        auto& projection = data_->Projection;
        if (isProjectionDirty_)
        {
            projection = DirectX::XMMatrixPerspectiveFovLH(fov_, aspectRatio_, nearZ_, farZ_);
            isProjectionDirty_ = false;
        }
        return projection;
    }

    DirectX::XMFLOAT3 Camera::GetEyePos() const noexcept
    {
        return { position_.x, position_.y, position_.z };
    }

    void Camera::Walk(float d) noexcept
    {
        Translate(0.0f, 0.0f, d, Space::LocalSpace);
    }

    void Camera::Strafe(float d) noexcept
    {
        Translate(d, 0.0f, 0.0f, Space::LocalSpace);
    }
}

