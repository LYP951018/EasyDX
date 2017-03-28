#pragma once

#include "DXDef.hpp"
#include <memory>

namespace dx
{
    //poor man's scene
    class Camera;

    class Scene
    {
        friend class GameWindow;
        friend class Game;

    public:
        virtual ~Scene();

        Camera& GetMainCamera() const noexcept;
        void SetMainCamera(std::unique_ptr<Camera> mainCamera) noexcept;

    protected:
        virtual void Start(ID3D11Device&);
        virtual void Render(ID3D11DeviceContext&, ID2D1DeviceContext&) = 0;
        virtual void Destroy() noexcept;

    private:
        std::unique_ptr<Camera> mainCamera_;
    };
}