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
        //FIXEME.
        GetGame().GetMainWindow()->Clear(DirectX::Colors::White);
        context.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        const auto& renderable = *GetComponent<Renderable>(object);
        renderable.Cb->Flush(context);
        SetupShader(context, renderable.VS.Get());
        SetupShader(context, renderable.PS.Get());
        auto gpuMesh = renderable.Mesh.Get();
        SetupGpuMesh(context, gpuMesh);
        context.DrawIndexed(gpuMesh.IndexCount, 0, 0);
    }

    void BehaviorSystem(GameObject& object, const UpdateArgs& args)
    {
        ranges::for_each(object.GetBehaviors(), [&](Behavior& behavior) { behavior.Update(object, args); });
    }
}
