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
        std::vector<Rc<IConstantBuffer>> CpuCbs;

        Renderable(VertexShader vs, PixelShader ps, GpuMesh mesh, std::vector<Rc<IConstantBuffer>> cbs)
            : VS{std::move(vs)},
            PS{std::move(ps)},
            Mesh{std::move(mesh)},
            CpuCbs{std::move(cbs)}
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