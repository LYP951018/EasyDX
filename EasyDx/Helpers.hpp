#pragma once

namespace dx
{
    class Camera;

    Rc<GameObject> MakeBasicGameObject(ID3D11Device& device, 
        const Predefined& predefined, 
        gsl::span<const SimpleVertex> vertices, 
        gsl::span<const std::uint16_t> indices,
        Rc<Smoothness> smoothness,
        Rc<Texture> texture = {});
}