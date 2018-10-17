#include "pch.hpp"
#include "Camera.hpp"
#include "Resources/Buffers.hpp"
#include <d3d11.h>
#include <Windows.h>
#include "Game.hpp"
#include "GameWindow.hpp"

namespace dx
{
    namespace Internal
    {
        struct alignas(16) CameraData
        {
            DirectX::XMMATRIX View;
            DirectX::XMMATRIX Projection;
            DirectX::BoundingFrustum Frustum;
        };
    }

    Camera::Camera()
        : m_isProjectionDirty{ true },
        m_isViewDirty{ true },
        rotation_{ 0.f, 0.f, 0.f, 1.f },
        position_{ 0.f, 0.f, 0.f, 1.f },
        data_{ aligned_unique<Internal::CameraData>() },
        m_viewport{ std::make_unique<Rect>() },
        m_defaultMove{}
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
        m_isViewDirty = true;
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
        m_isViewDirty = true;
    }

    void Camera::PrepareForRendering(ID3D11DeviceContext& context3D, const Game& game)
    {
        auto[width, height] = game.MainWindow().GetSize();
        const auto vpWidth = m_viewport->Right * static_cast<float>(width) - m_viewport->Left;
        const auto vpHeight = m_viewport->Bottom * static_cast<float>(height) - m_viewport->Top;
        const auto d3dViewport = D3D11_VIEWPORT{
            m_viewport->Left,
            m_viewport->Top,
            vpWidth,
            vpHeight,
            0.0f, 1.0f
        };
        context3D.RSSetViewports(1, &d3dViewport);
    }

    void Camera::Update(const UpdateArgs&, const Game & game)
    {
        if (m_defaultMove)
        {
            auto& inputSystem = game.GetInputSystem();
            if (inputSystem.IsPressing(VirtualKey::kLeftButton))
            {
                auto[x, y] = inputSystem.MouseMoved();
                const float dx = DirectX::XMConvertToRadians(0.25f * static_cast<float>(x));
                const float dy = DirectX::XMConvertToRadians(0.25f * static_cast<float>(y));
                RotateY(dx);
                RotateX(dy);
            }
            if (inputSystem.IsPressing(VirtualKey::kUp))
            {
                Walk(1.0f);
            }
            if (inputSystem.IsPressing(VirtualKey::kDown))
            {
                Walk(-1.0f);
            }
            if (inputSystem.IsPressing(VirtualKey::kLeft))
            {
                Strafe(-1.0f);
            }
            if (inputSystem.IsPressing(VirtualKey::kRight))
            {
                Strafe(1.0f);
            }
        }
    }

    void Camera::OnResize(Size newSize)
    {
        SetProjection(Fov(), newSize.GetAspectRatio(), NearZ(), FarZ());
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
        m_isProjectionDirty = true;
        m_fov = fov;
        aspectRatio_ = aspectRatio;
        m_nearZ = nearZ;
        m_farZ = farZ;
    }

    void Camera::SetLookAt(const DirectX::XMFLOAT3& eye, const DirectX::XMFLOAT3& target, const DirectX::XMFLOAT3& up) noexcept
    {
        const auto view = DirectX::XMMatrixLookAtLH(DirectX::XMLoadFloat3(&eye),
            DirectX::XMLoadFloat3(&target),
            DirectX::XMLoadFloat3(&up));
        position_ = { eye.x, eye.y, eye.z, 1.f };
        DirectX::XMStoreFloat4(&rotation_, DirectX::XMQuaternionRotationMatrix(XMMatrixTranspose(view)));
        m_isViewDirty = true;
    }

    void Camera::UpdateAspectRatio(float aspectRatio) noexcept
    {
        m_isProjectionDirty = true;
        aspectRatio_ = aspectRatio;
    }

    bool Camera::HasChanged() const noexcept
    {
        return m_isViewDirty || m_isProjectionDirty;
    }

    DirectX::XMMATRIX Camera::GetView() const noexcept
    {
        auto& view = data_->View;
        if (m_isViewDirty)
        {
            auto rotation = XMMatrixTranspose(DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&rotation_)));
            auto translation = DirectX::XMMatrixTranslation(-position_.x, -position_.y, -position_.z);
            view = translation * rotation;
            m_isViewDirty = false;
        }
        return view;
    }

    DirectX::XMMATRIX Camera::GetProjection() const noexcept
    {
        auto& projection = data_->Projection;
        if (m_isProjectionDirty)
        {
            projection = DirectX::XMMatrixPerspectiveFovLH(Fov(), aspectRatio_, NearZ(), FarZ());
            m_isProjectionDirty = false;
        }
        return projection;
    }

    DirectX::XMFLOAT3 Camera::GetEyePos() const noexcept
    {
        return { position_.x, position_.y, position_.z };
    }

    void Camera::UseDefaultMoveEvents(bool use)
    {
        m_defaultMove = use;
    }

    void Camera::Walk(float d) noexcept
    {
        Translate(0.0f, 0.0f, d, Space::LocalSpace);
    }

    void Camera::Strafe(float d) noexcept
    {
        Translate(d, 0.0f, 0.0f, Space::LocalSpace);
    }

    const DirectX::BoundingFrustum& Camera::Frustum() const
    {
        auto& frustum = data_->Frustum;
        if (m_isProjectionDirty)
        {
            const auto tanYFov = std::tan(Fov());
            const auto tanXFov = tanYFov * aspectRatio_;
            frustum.Origin = GetEyePos();
            frustum.Orientation = rotation_;
            frustum.RightSlope = tanXFov;
            frustum.LeftSlope = -tanXFov;
            frustum.TopSlope = tanYFov;
            frustum.BottomSlope = -tanYFov;
            frustum.Near = NearZ();
            frustum.Far = FarZ();
        }
        return frustum;
    }
}

