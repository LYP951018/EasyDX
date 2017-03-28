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

    class Camera
    {
    public:
        Camera() noexcept;

        void Translate(float x, float y, float z, Space space = Space::WorldSpace) noexcept;
        void Rotate(DirectX::XMVECTOR rotation) noexcept;
        void SetProjection(float fov, float aspectRatio, float nearZ, float farZ) noexcept;
        void SetLookAt(const DirectX::XMFLOAT3& eye, const DirectX::XMFLOAT3& target, const DirectX::XMFLOAT3& up) noexcept;
        void UpdateAspectRatio(float aspectRatio) noexcept;
        bool HasChanged() const noexcept;
        DirectX::XMMATRIX GetView() const noexcept;
        DirectX::XMMATRIX GetProjection() const noexcept;

        Viewport MainViewport;

    private:
        DirectX::XMVECTOR LoadTranslation() const noexcept;
        DirectX::XMVECTOR LoadRotation() const noexcept;

        float fov_;
        float aspectRatio_;
        float nearZ_, farZ_;

        mutable bool isProjectionDirty_, isViewDirty_;
        DirectX::XMFLOAT4 rotation_;
        DirectX::XMFLOAT4 translation_;
        mutable DirectX::XMFLOAT4X4 view_, projection_;
    };
}