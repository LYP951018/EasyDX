#pragma once

#include "DXDef.hpp"
#include <memory>
#include <chrono>

namespace dx
{
    //poor man's scene
    class Camera;

    struct UpdateArgs
    {
        std::chrono::milliseconds DeltaTime;
        std::chrono::milliseconds TotalTime;
        ID3D11DeviceContext& Context3D;
        ID2D1DeviceContext& Context2D;
    };

    class Scene
    {
        friend class GameWindow;
        friend class Game;

    public:
        virtual ~Scene();
        Scene();
        Camera& GetMainCamera() const noexcept;
        void SetMainCamera(std::unique_ptr<Camera> mainCamera) noexcept;

    protected:
        virtual void Start(ID3D11Device&);
        virtual void Update(const UpdateArgs& updateArgs);
        virtual void Render(ID3D11DeviceContext&, ID2D1DeviceContext&) = 0;
        virtual void Destroy() noexcept;

    private:
        std::unique_ptr<Camera> mainCamera_;
    };
}