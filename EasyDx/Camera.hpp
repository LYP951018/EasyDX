#pragma once

#include <DirectXMath.h>

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

    //TODO: view matrix caching.
    class Camera
    {
    public:
        Camera() noexcept = default;

        //Why copy a camera?
        Camera(const Camera&) = delete;
        Camera& operator= (const Camera&) = delete;

        void Translate(DirectX::XMVECTOR translation, Space space = Space::LocalSpace) noexcept;
        void Rotate(DirectX::XMVECTOR rotation) noexcept;

        DirectX::XMVECTOR GetTranslation() const noexcept;
        DirectX::XMVECTOR GetRotation() const noexcept;

        DirectX::XMMATRIX GetView() const noexcept;
        DirectX::XMMATRIX GetProjection() const noexcept;

        void SetProjection(float fov, float aspectRatio, float nearZ, float farZ) noexcept;
        void SetUvn(const DirectX::XMFLOAT3& eye, const DirectX::XMFLOAT3& target, const DirectX::XMFLOAT3& up) noexcept;

        void UpdateAspectRatio(float aspectRatio) noexcept;

        Viewport MainViewport;

    private:
        float fov_;
        float aspectRatio_;
        float nearZ_, farZ_;

        mutable bool isProjectionDirty_{};

        mutable DirectX::XMFLOAT4X4 viewMatrix_;
        mutable DirectX::XMFLOAT4X4 projectionMatrix_;
        DirectX::XMFLOAT4 rotation_{ 0.f, 0.f, 0.f, 1.f };
        DirectX::XMFLOAT4 translation_{ 0.f, 0.f, 0.f, 1.f };
    };
}