#pragma once

#include <DirectXMath.h>
#include <cstdlib>

namespace dx
{
    struct Viewport
    {
        float TopLeftX, TopLeftY, Width, Height, MinDepth, MaxDepth;
    };

    enum class Space
    {
        LocalSpace,
        WorldSpace
    };

    namespace Internal
    {
        struct alignas(16) CameraData;
    }

    class Camera
    {
    public:
        Camera();

        void Translate(float x, float y, float z, Space space = Space::LocalSpace) noexcept;
        void Rotate(DirectX::XMVECTOR rotation) noexcept;
        void SetProjection(float fov, float aspectRatio, float nearZ, float farZ) noexcept;
        void SetLookAt(const DirectX::XMFLOAT3& eye, const DirectX::XMFLOAT3& target, const DirectX::XMFLOAT3& up) noexcept;
        void UpdateAspectRatio(float aspectRatio) noexcept;
        bool HasChanged() const noexcept;
        DirectX::XMMATRIX GetView() const noexcept;
        DirectX::XMMATRIX GetProjection() const noexcept;
        DirectX::XMFLOAT3 GetEyePos() const noexcept;
        void Walk(float d) noexcept;

        Viewport MainViewport;

    private:
        DirectX::XMVECTOR LoadTranslation() const noexcept;
        DirectX::XMVECTOR LoadRotation() const noexcept;

        float fov_;
        float aspectRatio_;
        float nearZ_, farZ_;

        mutable bool isProjectionDirty_, isViewDirty_;
        DirectX::XMFLOAT4 rotation_;
        DirectX::XMFLOAT4 position_;
        aligned_unique_ptr<Internal::CameraData> data_;
    };
}