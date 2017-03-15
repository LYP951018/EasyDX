#pragma once

#include "DXDef.hpp"
#include "Camera.hpp"
#include <memory>

namespace dx
{
    //poor man's scene
    class Scene
    {
        friend class GameWindow;
        friend class Game;

    public:
        Scene() = default;

        Scene(const Scene&) = delete;
        Scene& operator= (const Scene&) = delete;

        Camera& GetMainCamera() const noexcept;
        void SetMainCamera(std::unique_ptr<Camera> mainCamera) noexcept;

    protected:
        virtual void Start();
        virtual void Render(ID3D11DeviceContext&, ID2D1DeviceContext&) = 0;
        virtual void Destroy() noexcept;

    private:
        std::unique_ptr<Camera> mainCamera_;
    };
}