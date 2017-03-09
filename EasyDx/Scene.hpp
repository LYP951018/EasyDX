#pragma once

#include "DXDef.hpp"

namespace dx
{
    //poor man's scene
    class Scene
    {
        friend class GameWindow;
        friend class Game;
    protected:
        virtual void Start();
        virtual void Render(ID3D11DeviceContext&, ID2D1DeviceContext&) = 0;
    };
}