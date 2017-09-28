#include "pch.hpp"
#include "Systems.hpp"
#include "Shaders.hpp"
#include "Renderable.hpp"
#include "Game.hpp"
#include "GameWindow.hpp"
#include <d3d11.h>
#include <DirectXColors.h>

namespace dx
{
    void RenderSystem(ID3D11DeviceContext& context, const GameObject& object)
    {
        context.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        const auto& renderable = *GetComponent<Renderable>(object);
        for (const auto& cb : renderable.CpuCbs)
        {
            cb->Flush(context);
        }
        SetupShader(context, renderable.VS.Get());
        SetupShader(context, renderable.PS.Get());
        auto gpuMesh = renderable.Mesh.Get();
        SetupGpuMesh(context, gpuMesh);
        context.DrawIndexed(gpuMesh.IndexCount, 0, 0);
    }

    void BehaviorSystem(GameObject& object, const UpdateArgs& args, int time)
    {
        ranges::for_each(object.GetBehaviors(), [&](Behavior& behavior) { if (behavior.GetExeTime() == time) behavior.Update(object, args); });
    }

    void BasicSystem(ID3D11DeviceContext& context, const UpdateArgs& args, GameObject& object)
    {
        BehaviorSystem(object, args, Behavior::Time::kCbUpdate);
        RenderSystem(context, object);
    }
}
