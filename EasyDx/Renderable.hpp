#pragma once

#include "Component.hpp"
#include "CBStructs.hpp"
#include "Mesh.hpp"
#include "GameObject.hpp"
#include "Shaders.hpp"

namespace dx
{
    struct Renderable : IComponent
    {
        VertexShader VS;
        PixelShader PS;
        GpuMesh Mesh;
        Rc<IConstantBuffer> Cb;

        Renderable(VertexShader vs, PixelShader ps, GpuMesh mesh, Rc<IConstantBuffer> cb)
            : VS{std::move(vs)},
            PS{std::move(ps)},
            Mesh{std::move(mesh)},
            Cb{std::move(cb)}
        {}

        std::uint32_t GetId() const override
        {
            return ComponentId::kRenderable;
        }

        static std::uint32_t GetStaticId()
        {
            return ComponentId::kRenderable;
        }
    };
}