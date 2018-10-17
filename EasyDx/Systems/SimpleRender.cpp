#include "../pch.hpp"
#include "SimpleRender.hpp"
#include "../Object.hpp"
#include "../Mesh.hpp"
#include "../Material.hpp"
#include "../MeshRenderer.hpp"
#include "../Scene.hpp"
#include "../Transform.hpp"
#include "../CBStructs.hpp"
#include "../Render.hpp"
#include "../Resources/Shaders.hpp"

namespace dx::systems
{
    void PrepareVsCb(ID3D11DeviceContext& context3D, ShaderInputs& inputs,
                     const DirectX::XMMATRIX& world,
                     [[maybe_unused]] gsl::span<const dx::Light> lights, const dx::Camera& camera)
    {
        // TODO: input validation.
        auto& vsCb = *inputs.GetCbInfo("TransformMatrices");
        vsCb.Set("WorldViewProj", world * camera.GetView() * camera.GetProjection());
        vsCb.Set("World", world);
        vsCb.Set("WorldInvTranspose",
                 DirectX::XMMatrixInverse(nullptr, DirectX::XMMatrixTranspose(world)));
    }

    void PreparePsCb(ID3D11DeviceContext& context3D, ShaderInputs& inputs,
                     gsl::span<const dx::Light> lights, const dx::Camera& camera)
    {
        auto& globalLights = *inputs.GetCbInfo("GlobalLightingInfo");
        globalLights.Set("EyePos", camera.GetEyePos());
        globalLights.Set("LightCount", static_cast<std::int32_t>(lights.size()));
        auto& lightCbs = globalLights.BorrowMut<dx::cb::Light[10]>("Lights");
        std::copy(lights.begin(), lights.end(), lightCbs);
    }

    void PrepareForRendering(ID3D11DeviceContext& context3D, gsl::span<const Light> lights,
                             const Camera& camera, Material& material,
                             const DirectX::XMMATRIX& world)
    {
        auto& shaders = material.Passes[0].Shaders;
        PrepareVsCb(context3D, shaders.VertexShader_.Inputs, world, lights, camera);
        PreparePsCb(context3D, shaders.PixelShader_.Inputs, lights, camera);
    }

    void SimpleRenderSystem(ID3D11DeviceContext& context3D, const SceneBase& scene,
                            const Object& object)
    {
        const auto meshRenderer = object.GetComponent<MeshRenderer>();
        if (meshRenderer == nullptr)
            return;
        PrepareForRendering(context3D, gsl::make_span(scene.Lights()), scene.MainCamera(),
                            meshRenderer->GetMaterial(),
                            object.GetComponent<TransformComponent>()->GetTransform().Matrix());
        DrawMesh(context3D, meshRenderer->GetMesh(), meshRenderer->GetMaterial());
    }
} // namespace dx::systems
