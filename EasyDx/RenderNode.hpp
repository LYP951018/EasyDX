#pragma once

#include <DirectXMath.h>

namespace dx
{
    class Mesh;
    class Material;
    class GlobalShaderContext;
    class CallbackComponent;
    class Pass;
    class ShaderInputs;

    struct RenderNode
    {
        Mesh& mesh;
        Material& material;
        DirectX::XMMATRIX World;
        CallbackComponent* const renderCallbacks;
    };
}