#pragma once

#include <DirectXMath.h>
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
        void UseDefaultMoveEvents(bool use);
        void Walk(float d) noexcept;
        void Strafe(float d) noexcept;

        Rect& Viewport() noexcept { return *m_viewport; }
        const Rect& Viewport() const noexcept { return *m_viewport; }

        float Fov;
        float NearZ, FarZ;

    private:
        friend class Scene;
        friend class Game;
        void PrepareForRendering(ID3D11DeviceContext& context3D, const Game& game);
        void Update(const UpdateArgs& args, const Game& game);
        void OnResize(Size newSize);

        DirectX::XMVECTOR LoadTranslation() const noexcept;
        DirectX::XMVECTOR LoadRotation() const noexcept;

        float aspectRatio_;
        mutable bool isProjectionDirty_, isViewDirty_;
        bool m_defaultMove;
        DirectX::XMFLOAT4 rotation_;
        DirectX::XMFLOAT4 position_;
        aligned_unique_ptr<Internal::CameraData> data_;
        std::unique_ptr<Rect> m_viewport;
    };

}