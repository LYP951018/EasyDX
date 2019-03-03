#include "pch.hpp"
#include "Material.hpp"

namespace dx
{
    void SetupBlending(ID3D11DeviceContext& context3D, const BlendSettings& blendSettings)
    {
        if (blendSettings.BlendState == nullptr)
        {
            context3D.OMSetBlendState(nullptr, nullptr, UINT32_MAX);
        }
        else
        {
            context3D.OMSetBlendState(blendSettings.BlendState.Get(),
                                      blendSettings.BlendFactor.data(), blendSettings.SampleMask);
        }
    }

    void SetupDepthStencilStates(ID3D11DeviceContext& context3D,
                                 const DepthStencilSettings& depthStencilSettings)
    {
        if (depthStencilSettings.StencilState == nullptr)
        {
            context3D.OMSetDepthStencilState(nullptr, 0);
        }
        else
        {
            context3D.OMSetDepthStencilState(depthStencilSettings.StencilState.Get(),
                                             depthStencilSettings.StencilRef);
        }
    }

    void SetupRasterizerState(ID3D11DeviceContext& context3D, ID3D11RasterizerState& rasterState)
    {
        context3D.RSSetState(&rasterState);
    }

    void SetupPass(ID3D11DeviceContext& context3D, const Pass& pass)
    {
        SetupShaders(context3D, pass.Shaders);
        SetupBlending(context3D, pass.Blending);
        SetupDepthStencilStates(context3D, pass.DepthStencil);
        SetupRasterizerState(context3D, dx::Ref(pass.RasterizerState));
    }
    void PredefinedPasses::Initialize() {}

    std::shared_ptr<Pass> PredefinedPasses::GetPlainShadowCaster()
    {
        return std::shared_ptr<Pass>();
    }
} // namespace dx