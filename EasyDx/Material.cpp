#include "Material.hpp"
#include <d3d11.h>

namespace dx
{
    void SetupTextures(ID3D11DeviceContext& context, const Material& material)
    {
        if (material.UseTexture && !material.Textures.empty())
        {
            assert(!material.Samplers.empty() && "samplers should not be empty");
            const auto textures = ComPtrsCast(gsl::make_span(material.Textures));
            context.PSSetShaderResources(0, textures.size(), textures.data());
            const auto samplers = ComPtrsCast(gsl::make_span(material.Samplers));
            context.PSSetSamplers(0, samplers.size(), samplers.data());
        }
    }
}
