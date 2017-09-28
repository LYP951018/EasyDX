#include "pch.hpp"
#include "Helpers.hpp"
#include "Camera.hpp"
#include "Predefined.hpp"
#include "SimpleVertex.hpp"
#include "Renderable.hpp"
#include "CBStructs.hpp"

namespace dx
{
    Rc<GameObject> MakeBasicGameObject(ID3D11Device& device, 
        const Predefined& predefined,
        gsl::span<const SimpleVertex> vertices,
        gsl::span<const std::uint16_t> indices,
        Rc<Smoothness> smoothness,
        Rc<Texture> texture)
    {
        auto object = MakeShared<GameObject>();
        auto vs = predefined.GetBasicVS();
        auto ps = predefined.GetBasicPS();
        auto renderable = MakeRenderable(std::move(vs), std::move(ps),
            GpuMesh{ device, SimpleCpuMeshView{ vertices, indices } }, 
            MakeCbs(predefined.GetBasicVSCpuCb(), predefined.GetBasicLightingCpuCb()));
        object->AddComponents(std::move(renderable), std::move(smoothness));
        object->AddBehaviors(predefined.GetBasicCbUpdator());
        object->AddComponent(texture ? texture : predefined.GetWhite());
        return object;
    }
}

