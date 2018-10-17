#pragma once

#include "../Light.hpp"
#include <DirectXMath.h>

namespace dx
{
    class Object;
    class Camera;
    class SceneBase;
    class ShaderInputs;

    namespace systems
    {
        void PrepareVsCb(ID3D11DeviceContext& context3D, ShaderInputs& inputs,
                         const DirectX::XMMATRIX& world,
                         gsl::span<const dx::Light> lights,
                         const dx::Camera& camera);

        void PreparePsCb(ID3D11DeviceContext& context3D, ShaderInputs& inputs,
                         gsl::span<const dx::Light> lights, const dx::Camera& camera);

        void SimpleRenderSystem(ID3D11DeviceContext& context3D, const SceneBase& scene, const Object& object);
    }
}