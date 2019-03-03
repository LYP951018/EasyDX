#include "pch.hpp"
#include <d3d11.h>
#include "Predefined.hpp"
#include "Camera.hpp"
#include "Material.hpp"
#include "CBStructs.hpp"
#include "Texture.hpp"
#include "BasicVS.hpp"
#include "BasicPS.hpp"
#include "FakePosVS.hpp"
#include "FakePosNormalVS.hpp"
#include "FakePosNormalTexVS.hpp"
#include "FakePosNormTanTexVS.hpp"
#include "Bind.hpp"
#include "Mesh.hpp"
#include "Material.hpp"
#include "MeshRenderer.hpp"
#include "Transform.hpp"
#include "Model.hpp"

namespace dx
{
    using gsl::span;

    PredefinedResources::PredefinedResources(ID3D11Device& device)
    {
        MakeWhiteTex(device);
        MakeStencilStates(device);
        MakeBlendingStates(device);
        MakeRasterizerStates(device);
    }

    std::unique_ptr<const PredefinedResources> g_predefineResources;

    PredefinedResources::~PredefinedResources() {}

    wrl::ComPtr<ID3D11ShaderResourceView> PredefinedResources::GetWhite()
    {
        return g_predefineResources->white_;
    }

    wrl::ComPtr<ID3D11DepthStencilState> PredefinedResources::GetStencilAlways()
    {
        return g_predefineResources->stencilAlways_;
    }

    wrl::ComPtr<ID3D11DepthStencilState> PredefinedResources::GetDrawnOnly()
    {
        return g_predefineResources->drawnOnly_;
    }

    wrl::ComPtr<ID3D11DepthStencilState> PredefinedResources::GetNoDoubleBlending()
    {
        return g_predefineResources->noDoubleBlending_;
    }

    wrl::ComPtr<ID3D11BlendState> PredefinedResources::GetNoWriteToRT()
    {
        return g_predefineResources->noWriteToRt_;
    }

    wrl::ComPtr<ID3D11BlendState> PredefinedResources::GetTransparent()
    {
        return g_predefineResources->transparent_;
    }

    ID3D11RasterizerState* PredefinedResources::GetCullClockwise()
    {
        return g_predefineResources->cullClockWise_.Get();
    }

    ID3D11RasterizerState* PredefinedResources::GetWireFrameOnly()
    {
        return g_predefineResources->wireFrameOnly_.Get();
    }

    wrl::ComPtr<ID3D11SamplerState> PredefinedResources::GetDefaultSampler()
    {
        return g_predefineResources->defaultSampler_;
    }

    wrl::ComPtr<ID3D11SamplerState> PredefinedResources::GetRepeatSampler()
    {
        return g_predefineResources->repeatSampler_;
    }

    wrl::ComPtr<ID3D11SamplerState> PredefinedResources::GetShadowMapSampler()
    {
        return g_predefineResources->m_shadowMapSampler;
    }

    void PredefinedResources::MakeWhiteTex(ID3D11Device& device)
    {
        wrl::ComPtr<ID3D11Texture2D> tex;
        {
            auto pixels = std::vector<DirectX::XMFLOAT4>(kDefaultTexHeight * kDefaultTexWidth);
            std::fill(pixels.begin(), pixels.end(), DirectX::XMFLOAT4{1.0f, 1.0f, 1.0f, 1.0f});
            CD3D11_TEXTURE2D_DESC desc{DXGI_FORMAT_R32G32B32A32_FLOAT, kDefaultTexWidth,
                                       kDefaultTexHeight};
            desc.MipLevels = 1;
            D3D11_SUBRESOURCE_DATA resourceData{};
            resourceData.pSysMem = pixels.data();
            resourceData.SysMemPitch = kDefaultTexWidth;
            resourceData.SysMemSlicePitch =
                gsl::narrow<UINT>(pixels.size() * sizeof(DirectX::XMFLOAT4));
            TryHR(device.CreateTexture2D(&desc, &resourceData, tex.GetAddressOf()));
        }
        {
            CD3D11_SAMPLER_DESC desc{CD3D11_DEFAULT{}};
            // desc.Filter = D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
            TryHR(device.CreateSamplerState(&desc, defaultSampler_.GetAddressOf()));
        }
        {
            CD3D11_SAMPLER_DESC desc{CD3D11_DEFAULT{}};
            desc.AddressW = desc.AddressV = desc.AddressU =
                D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_WRAP;
            TryHR(device.CreateSamplerState(&desc, repeatSampler_.GetAddressOf()));
        }
        {
            D3D11_SAMPLER_DESC desc{};
            desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
            desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
            desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
            desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
            desc.ComparisonFunc = D3D11_COMPARISON_LESS;
            TryHR(device.CreateSamplerState(&desc, m_shadowMapSampler.GetAddressOf()));
        }
        white_ = Get2DTexView(device, Ref(tex));
    }

    void PredefinedResources::MakeStencilStates(ID3D11Device& device)
    {
        {
            D3D11_DEPTH_STENCIL_DESC stencilDesc{};
            //采用正常的 depth test。
            stencilDesc.DepthEnable = true;
            stencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
            stencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

            stencilDesc.StencilEnable = true;
            stencilDesc.StencilReadMask = 0xff;
            stencilDesc.StencilWriteMask = 0xff;

            stencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
            stencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
            stencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
            stencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

            stencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
            stencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
            stencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
            stencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

            TryHR(device.CreateDepthStencilState(&stencilDesc, stencilAlways_.GetAddressOf()));
        }

        {
            D3D11_DEPTH_STENCIL_DESC drawReflectionDesc;
            drawReflectionDesc.DepthEnable = true;
            drawReflectionDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
            drawReflectionDesc.DepthFunc = D3D11_COMPARISON_LESS;
            drawReflectionDesc.StencilEnable = true;
            drawReflectionDesc.StencilReadMask = 0xff;
            drawReflectionDesc.StencilWriteMask = 0xff;

            drawReflectionDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
            drawReflectionDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
            drawReflectionDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
            drawReflectionDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;

            drawReflectionDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
            drawReflectionDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
            drawReflectionDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
            drawReflectionDesc.BackFace.StencilFunc = D3D11_COMPARISON_EQUAL;

            TryHR(device.CreateDepthStencilState(&drawReflectionDesc, drawnOnly_.GetAddressOf()));
        }

        {
            D3D11_DEPTH_STENCIL_DESC noDoubleBlendDesc{};
            noDoubleBlendDesc.DepthEnable = true;
            noDoubleBlendDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
            noDoubleBlendDesc.DepthFunc = D3D11_COMPARISON_LESS;
            noDoubleBlendDesc.StencilEnable = true;
            noDoubleBlendDesc.StencilReadMask = 0xff;
            noDoubleBlendDesc.StencilWriteMask = 0xff;

            noDoubleBlendDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
            noDoubleBlendDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
            noDoubleBlendDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_INCR;
            noDoubleBlendDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;

            noDoubleBlendDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
            noDoubleBlendDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
            noDoubleBlendDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_INCR;
            noDoubleBlendDesc.BackFace.StencilFunc = D3D11_COMPARISON_EQUAL;

            TryHR(device.CreateDepthStencilState(&noDoubleBlendDesc,
                                                 noDoubleBlending_.GetAddressOf()));
        }
    }

    void PredefinedResources::MakeBlendingStates(ID3D11Device& device)
    {
        {
            D3D11_BLEND_DESC noRenderTargetWritesDesc = {};
            noRenderTargetWritesDesc.AlphaToCoverageEnable = false;
            noRenderTargetWritesDesc.IndependentBlendEnable = false;

            noRenderTargetWritesDesc.RenderTarget[0].BlendEnable = false;
            noRenderTargetWritesDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
            noRenderTargetWritesDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
            noRenderTargetWritesDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
            noRenderTargetWritesDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
            noRenderTargetWritesDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
            noRenderTargetWritesDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
            noRenderTargetWritesDesc.RenderTarget[0].RenderTargetWriteMask = 0;

            TryHR(device.CreateBlendState(&noRenderTargetWritesDesc, noWriteToRt_.GetAddressOf()));
        }

        {
            D3D11_BLEND_DESC transparentDesc = {};
            transparentDesc.AlphaToCoverageEnable = false;
            transparentDesc.IndependentBlendEnable = false;

            transparentDesc.RenderTarget[0].BlendEnable = true;
            transparentDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_BLEND_FACTOR;
            transparentDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_BLEND_FACTOR;
            transparentDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
            transparentDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
            transparentDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
            transparentDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
            transparentDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

            TryHR(device.CreateBlendState(&transparentDesc, transparent_.GetAddressOf()));
        }
    }

    void PredefinedResources::MakeRasterizerStates(ID3D11Device& device)
    {
        {
            D3D11_RASTERIZER_DESC cullClockwiseDesc{};
            cullClockwiseDesc.FillMode = D3D11_FILL_SOLID;
            cullClockwiseDesc.CullMode = D3D11_CULL_BACK;
            cullClockwiseDesc.FrontCounterClockwise = true;
            cullClockwiseDesc.DepthClipEnable = true;
            TryHR(device.CreateRasterizerState(&cullClockwiseDesc, cullClockWise_.GetAddressOf()));
        }

        {
            D3D11_RASTERIZER_DESC wireframeDesc{};
            wireframeDesc.FillMode = D3D11_FILL_WIREFRAME;
            wireframeDesc.CullMode = D3D11_CULL_BACK;
            wireframeDesc.FrontCounterClockwise = false;
            wireframeDesc.DepthClipEnable = true;

            TryHR(device.CreateRasterizerState(&wireframeDesc, wireFrameOnly_.GetAddressOf()));
        }
    }

    std::unique_ptr<Object> obj(Object obj) { return std::make_unique<Object>(std::move(obj)); }

    void PresetupBasicPsCb(ShaderInputs& psInputs, const PredefinedResources& predefined,
                           const dx::Smoothness& smoothness,
                           wrl::ComPtr<ID3D11ShaderResourceView> mainTexture,
                           wrl::ComPtr<ID3D11SamplerState> sampler)
    {
        // auto& perObject = *psInputs.GetCbInfo("PerObjectLightingInfo");
        psInputs.SetField("ObjectMaterial", dx::cb::Material{smoothness, mainTexture != nullptr});
        if (mainTexture == nullptr)
        {
            mainTexture = predefined.GetWhite();
        }
        psInputs.Bind("Texture", std::move(mainTexture));
        if (sampler == nullptr)
        {
            sampler = predefined.GetDefaultSampler();
        }
        psInputs.Bind("Sampler", std::move(sampler));
    }

    std::shared_ptr<Material> MakeBasicLightingMaterial(
        const PredefinedResources& predefined, const dx::Smoothness& smoothness,
        wrl::ComPtr<ID3D11ShaderResourceView> mainTexture, wrl::ComPtr<ID3D11SamplerState> sampler)
    {
        Shader defaultVS = Shaders::Get(Shaders::kPosNormalTexTransform).value();
        Shader defaultPS = Shaders::Get(Shaders::kBasicLighting).value();
        ShaderCollection shaderCollection = MakeShaderCollection(
            std::move(defaultVS), std::move(defaultPS));
        // TODO: default pass should be shared
        std::shared_ptr<Pass> defaultPass =
            std::make_shared<dx::Pass>(dx::Pass{std::move(shaderCollection)});
        std::shared_ptr<Material> material =
            std::make_shared<Material>(Material{std::move(defaultPass)});
        PresetupBasicPsCb(material->mainPass.inputs, predefined, smoothness, std::move(mainTexture),
                          std::move(sampler));
		return material;
    }

    void PredefinedResources::Setup(ID3D11Device& device3D)
    {
        g_predefineResources = std::make_unique<const PredefinedResources>(device3D);
    }

    const PredefinedResources& PredefinedResources::GetInstance() { return *g_predefineResources; }

} // namespace dx
