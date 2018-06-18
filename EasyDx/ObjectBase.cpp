#include "pch.hpp"
#include "ObjectBase.hpp"

namespace dx
{
    ObjectBase::ObjectBase()
    {
        DirectX::XMStoreFloat4x4(&m_transform, DirectX::XMMatrixIdentity());
    }

    void ObjectBase::Update(const UpdateArgs &, const Game &)
    {
    }

    void ObjectBase::Render(ID3D11DeviceContext & context3D, const Game & game)
    {
    }

    ObjectBase::~ObjectBase()
    {
    }
}

