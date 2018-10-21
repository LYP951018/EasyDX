#pragma once

#include <DirectXMath.h>
#include <DirectXCollision.h>
#include "AlignedAllocator.hpp"
#include "Misc.hpp"

namespace dx
{
    enum class Space
    {
        LocalSpace,
        WorldSpace
    };

    namespace Internal
    {
        struct alignas(16) CameraData;
    }

    class Game;
    struct UpdateArgs;

    class Camera : Noncopyable
    {
    public:
        Camera();

        void Translate(float x, float y, float z, Space space = Space::LocalSpace) noexcept;
        void Pitch(float angle);
        void Yaw(float angle);
        void RotateY(float angle);
        void RotateX(float angle);
        void Rotate(DirectX::XMVECTOR rotation) noexcept;
        void SetProjection(float fov, float aspectRatio, float nearZ, float farZ) noexcept;
        void SetLookAt(const DirectX::XMFLOAT3& eye, const DirectX::XMFLOAT3& target, const DirectX::XMFLOAT3& up) noexcept;
        void UpdateAspectRatio(float aspectRatio) noexcept;
        bool HasChanged() const noexcept;
        DirectX::XMMATRIX GetView() const noexcept;
        DirectX::XMMATRIX GetProjection() const noexcept;
        DirectX::XMFLOAT3 GetEyePos() const noexcept;
        const DirectX::BoundingFrustum& Frustum() const;
        void UseDefaultMoveEvents(bool use);
        void Walk(float d) noexcept;
        void Strafe(float d) noexcept;

        Rect& Viewport() noexcept { return *m_viewport; }
        const Rect& Viewport() const noexcept { return *m_viewport; }

        //DirectX::XMMatrixPerspectiveFovLH 使用的是纵向 fov，这里也是纵向 fov。
        float Fov() const { return m_fov; }
        void SetFov(float fov)
        {
            m_fov = fov;
            m_isProjectionDirty = true;
        }

        float NearZ() const { return m_nearZ; }
        void SetNearZ(float nearZ)
        {
            m_nearZ = nearZ;
            m_isProjectionDirty = true;
        }

        float FarZ() const { return m_farZ; }
        void SetFarZ(float farZ)
        {
            m_farZ = farZ;
            m_isProjectionDirty = true;
        }

    private:
        friend class Scene;
        friend class Game;
        void PrepareForRendering(ID3D11DeviceContext& context3D, const Game& game);
        void Update(const UpdateArgs& args, const Game& game);
        void OnResize(Size newSize);
        void FlushProjectionDirty() const;

        DirectX::XMVECTOR LoadTranslation() const noexcept;
        DirectX::XMVECTOR LoadRotation() const noexcept;

        mutable bool m_isProjectionDirty, m_isViewDirty;
        bool m_defaultMove;
        float aspectRatio_;
        float m_fov, m_nearZ, m_farZ;
        DirectX::XMFLOAT4 rotation_;
        DirectX::XMFLOAT4 position_;
        aligned_unique_ptr<Internal::CameraData> data_;
        std::unique_ptr<Rect> m_viewport;
    };

}