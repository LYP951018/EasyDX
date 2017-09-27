#pragma once

#include "GameObject.hpp"
#include "Component.hpp"
#include "Common.hpp"
#include "Renderable.hpp"
#include "Predefined.hpp"
#include "CBStructs.hpp"
#include "Behavior.hpp"
#include <gsl/span>

namespace dx
{
    class Camera;

    template<typename VertexT, std::ptrdiff_t VertexN, std::ptrdiff_t IndexN, typename CbDataT>
    Rc<GameObject> MakeBasicGameObject(ID3D11Device& device, 
        const Predefined& predefined, 
        gsl::span<VertexT, VertexN> vertices, 
        gsl::span<const std::uint16_t, IndexN> indices,
        Rc<Cb<CbDataT>> cpuCb,
        GpuCb vsGpuCb,
        GpuCb globalLightCb,
        GpuCb perObjectLightingCb,
        Rc<Behavior> cbUpdator,
        Rc<Texture> texture)
    {
        auto object = MakeShared<GameObject>();
        auto vs = predefined.GetBasicVS();
        vs.Cbs.push_back(std::move(vsGpuCb));
        auto ps = predefined.GetBasicPS();
        ps.Cbs.push_back(std::move(globalLightCb));
        ps.Cbs.push_back(std::move(perObjectLightingCb));
        auto renderable = MakeShared<Renderable>(std::move(vs), std::move(ps), std::move(cpuCb),
            GpuMesh{device, CpuMeshView<VertexT>{vertices, indices}});
        object->AddComponent(std::move(renderable));
        object->AddBehaviors(std::move(cbUpdator));
        object->AddComponent(texture ? texture : predefined.GetWhite());
        return object;
    }

    struct BasicCbUpdator : Behavior
    {
        const Camera& Camera_;

        BasicCbUpdator(const Camera& camera)
            : Camera_{camera}, Behavior{Behavior::kCbUpdate}
        {}

        void Update(GameObject&, const UpdateArgs&) override;
    };
}