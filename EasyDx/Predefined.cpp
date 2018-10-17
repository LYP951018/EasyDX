#include "pch.hpp"
#include <d3d11.h>
#include "Predefined.hpp"
#include "Camera.hpp"
#include "Material.hpp"
#include "CBStructs.hpp"
#include "Texture.hpp"
#include <DirectXTK/DirectXHelpers.h>
#include "BasicVS.hpp"
#include "BasicPS.hpp"
#include "FakePosVS.hpp"
#include "FakePosNormalVS.hpp"
#include "FakePosNormalTexVS.hpp"
#include "Bind.hpp"
#include "Mesh.hpp"
#include "Material.hpp"
#include "MeshRenderer.hpp"
#include "Transform.hpp"

namespace dx
{
    using gsl::span;

    PredefinedResources::PredefinedResources(ID3D11Device& device)
        : basicVS_{device, AsBytes(BasicVertexShader)}, basicPS_{device, AsBytes(BasicPixelShader)}
    {
        MakeWhiteTex(device);
        MakeStencilStates(device);
        MakeBlendingStates(device);
        MakeRasterizerStates(device);
        MakeLayouts(device);
    }

    PredefinedResources::~PredefinedResources() {}

    wrl::ComPtr<ID3D11DepthStencilState> PredefinedResources::GetStencilAlways() const
    {
        return stencilAlways_;
    }

    wrl::ComPtr<ID3D11DepthStencilState> PredefinedResources::GetDrawnOnly() const
    {
        return drawnOnly_;
    }

    wrl::ComPtr<ID3D11DepthStencilState> PredefinedResources::GetNoDoubleBlending() const
    {
        return noDoubleBlending_;
    }

    wrl::ComPtr<ID3D11BlendState> PredefinedResources::GetNoWriteToRT() const
    {
        return noWriteToRt_;
    }

    wrl::ComPtr<ID3D11BlendState> PredefinedResources::GetTransparent() const
    {
        return transparent_;
    }

    wrl::ComPtr<ID3D11SamplerState> PredefinedResources::GetDefaultSampler() const
    {
        return defaultSampler_;
    }

    wrl::ComPtr<ID3D11SamplerState> PredefinedResources::GetRepeatSampler() const
    {
        return repeatSampler_;
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

    void PredefinedResources::MakeLayouts(ID3D11Device& device)
    {
        /*m_globalInputLayoutAllocator.Register(device, PosNormal,
                                              AsBytes(FakePosNormalVSByteCode));*/
        m_globalInputLayoutAllocator.Register(device, PosDesc, AsBytes(FakePosVSByteCode));
        m_globalInputLayoutAllocator.Register(device, PosNormalTexDescs, AsBytes(FakePosNormalTexVSByteCode));
    }

    std::unique_ptr<Object> obj(Object obj) { return std::make_unique<Object>(std::move(obj)); }

    std::shared_ptr<Material>
    MakeBasicLightingMaterial(const PredefinedResources& predefined,
                              wrl::ComPtr<ID3D11ShaderResourceView> mainTexture,
                              const dx::Smoothness& smoothness)
    {
        auto ps = predefined.GetBasicPS();
        auto& psInputs = ps.Inputs;
        auto& perObject = *psInputs.GetCbInfo("PerObjectLightingInfo");
        perObject.Set("ObjectMaterial", dx::cb::Material{smoothness, mainTexture != nullptr});
        if (mainTexture == nullptr)
        {
            mainTexture = predefined.GetWhite();
        }
        psInputs.Bind("Texture", std::move(mainTexture));
        // TODO
        psInputs.Bind("Sampler", predefined.GetDefaultSampler());
        return std::make_shared<Material>(Material{
            MakeVec(dx::Pass{dx::ShaderCollection{predefined.GetBasicVS(), std::move(ps)}})});
    }

    std::unique_ptr<dx::Object> MakeObjectWithDefaultRendering(
        ID3D11Device& device3D, const PredefinedResources& predefined,
        gsl::span<const dx::PositionType> positions, gsl::span<const dx::VectorType> normals,
        gsl::span<const dx::TexCoordType> texCoords, gsl::span<const dx::ShortIndex> indices,
        wrl::ComPtr<ID3D11ShaderResourceView> mainTexture, const dx::Smoothness& smoothness)
    {

        return obj(Object{MeshRenderer{
            Mesh::CreateImmutable(device3D, predefined.InputLayouts().Query(PosNormalTexDescs),
                                  span{indices}, span{positions}, span{normals}, span{texCoords}),
            MakeBasicLightingMaterial(predefined, std::move(mainTexture), smoothness)},
            //TODO: custom transform
            TransformComponent{Transform{}}});
    }
} // namespace dx
