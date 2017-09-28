#pragma once

#include "DXDef.hpp"

namespace dx
{
    class GameObject;
    class Predefined;
    struct SimpleVertex;
    struct Transformation;

    Rc<GameObject> MakeBasicGameObject(ID3D11Device& device, 
        const Predefined& predefined, 
        gsl::span<const SimpleVertex> vertices, 
        gsl::span<const std::uint16_t> indices,
        Rc<Smoothness> smoothness,
        const Transformation& transform = {},
        Rc<Texture> texture = {});
}