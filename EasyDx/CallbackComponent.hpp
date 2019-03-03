#pragma once

#include "ComponentBase.hpp"

namespace dx
{
    class GlobalShaderContext;
    class MeshRenderer;

    class CallbackComponent : public ComponentBase
    {
        // update cb, ...
        virtual void OnPrerender(const GlobalShaderContext& shaderContext,
                                 MeshRenderer& renderer) noexcept;
        virtual void OnPostrender() noexcept;
    };
} // namespace dx