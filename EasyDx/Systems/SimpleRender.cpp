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
    void PrepareVsCb(ID3D11DeviceContext&, ShaderInputs& inputs, const DirectX::XMMATRIX& world,
                     const DirectX::XMMATRIX& view, const DirectX::XMMATRIX& projection)
    {
        // TODO: input validation.
        // auto& vsCb = *inputs.GetCbInfo("TransformMatrices");
        /*inputs.SetField("WorldViewProj", world * view * projection);
        inputs.SetField("World", world);
        inputs.SetField("WorldInvTranspose",
                 DirectX::XMMatrixInverse(nullptr, DirectX::XMMatrixTranspose(world)));*/
    }

    void PreparePsCb(ID3D11DeviceContext& context3D, ShaderInputs& inputs,
                     gsl::span<const dx::Light> lights, const dx::Camera& camera)
    {
        // auto& globalLights = *inputs.GetCbInfo("GlobalLightingInfo");
        inputs.SetField("EyePos", camera.GetEyePos());
        inputs.SetField("LightCount", static_cast<std::int32_t>(lights.size()));
        auto& lightCbs = inputs.BorrowMut<dx::cb::Light[10]>("Lights");
        std::copy(lights.begin(), lights.end(), lightCbs);
    }

    void PrepareForRendering(ID3D11DeviceContext& context3D, gsl::span<const Light> lights,
                             const Camera& camera, Material& material,
                             const DirectX::XMMATRIX& world)
    {
        ShaderInputs& inputs = material.mainPass.inputs;
        PrepareVsCb(context3D, inputs, world, camera.GetView(), camera.GetProjection());
        PreparePsCb(context3D, inputs, lights, camera);
    }

    void SimpleRenderSystem(ID3D11DeviceContext& context3D, const SceneBase& scene,
                            const Object& object)
    {
        const auto meshRenderer = object.GetComponent<MeshRenderer>();
        if (meshRenderer == nullptr)
            return;
        const auto transform = object.GetComponent<TransformComponent>();
        PrepareForRendering(context3D, gsl::make_span(scene.Lights()), scene.MainCamera(),
                            meshRenderer->GetMaterial(),
                            transform == nullptr ? DirectX::XMMatrixIdentity()
                                                 : transform->GetTransform().Matrix());
        DrawMesh(context3D, meshRenderer->GetMesh(), meshRenderer->GetMaterial());
    }
} // namespace dx::systems
