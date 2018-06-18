#pragma once

#include <DirectXMath.h>

namespace dx
{
    struct UpdateArgs;
    class Game;

    class ObjectBase
    {
    public:
        ObjectBase();

        virtual void Update(const UpdateArgs&, const Game&);
        virtual void Render(ID3D11DeviceContext& context3D, const Game& game);

        virtual ~ObjectBase();

    protected:
        //TODO: a transform type
        DirectX::XMFLOAT4X4 m_transform;
    };
}