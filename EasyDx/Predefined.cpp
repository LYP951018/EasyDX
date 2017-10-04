#include "pch.hpp"
#include <d3d11.h>
#include "Predefined.hpp"
#include "Camera.hpp"
#include "Material.hpp"
#include "CBStructs.hpp"
#include "Texture.hpp"
#include <DirectXtk/DirectXHelpers.h>
#include "BasicVS.hpp"
#include "BasicPS.hpp"

namespace dx
{
    Predefined::Predefined(ID3D11Device& device)
    {
        MakeWhiteTex(device);
        MakeBasicVS(device);
        MakeBasicPS(device);
        MakeStencilStates(device);
        MakeBlendingStates(device);
        MakeRasterizerStates(device);
    }


    Predefined::~Predefined()
    {
    }

    wrl::ComPtr<ID3D11DepthStencilState> Predefined::GetStencilAlways() const { return stencilAlways_; }

    wrl::ComPtr<ID3D11DepthStencilState> Predefined::GetDrawnOnly() const { return drawnOnly_; }

    wrl::ComPtr<ID3D11DepthStencilState> Predefined::GetNoDoubleBlending() const
    {
        return noDoubleBlending_;
    }

    wrl::ComPtr<ID3D11BlendState> Predefined::GetNoWriteToRT() const { return noWriteToRt_; }

    wrl::ComPtr<ID3D11BlendState> Predefined::GetTransparent() const { return transparent_; }

    wrl::ComPtr<ID3D11SamplerState> Predefined::GetDefaultSampler() const { return defaultSampler_; }

    wrl::ComPtr<ID3D11SamplerState> Predefined::GetRepeatSampler() const
    {
        return repeatSampler_;
    }

    void Predefined::MakeWhiteTex(ID3D11Device& device)
    {
        wrl::ComPtr<ID3D11Texture2D> tex;
        {
            auto pixels = std::vector<DirectX::XMFLOAT4>(kDefaultTexHeight * kDefaultTexWidth);
            std::fill(pixels.begin(), pixels.end(), DirectX::XMFLOAT4{ 1.0f, 1.0f, 1.0f, 1.0f });
            CD3D11_TEXTURE2D_DESC desc{ DXGI_FORMAT_R32G32B32A32_FLOAT, kDefaultTexWidth, kDefaultTexHeight };
            desc.MipLevels = 1;
            D3D11_SUBRESOURCE_DATA resourceData{};
            resourceData.pSysMem = pixels.data();
            resourceData.SysMemPitch = kDefaultTexWidth;
            resourceData.SysMemSlicePitch = pixels.size() * sizeof(DirectX::XMFLOAT4);
            TryHR(device.CreateTexture2D(&desc, &resourceData, tex.GetAddressOf()));
        }
        {
            CD3D11_SAMPLER_DESC desc{ CD3D11_DEFAULT{} };
            //desc.Filter = D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
            TryHR(device.CreateSamplerState(&desc, defaultSampler_.GetAddressOf()));
        }
        {
            CD3D11_SAMPLER_DESC desc{ CD3D11_DEFAULT{} };
            desc.AddressW = desc.AddressV = desc.AddressU = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_WRAP;
             D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_WRAP;
            TryHR(device.CreateSamplerState(&desc, repeatSampler_.GetAddressOf()));
        }
        white_ = Get2DTexView(device, Ref(tex));
    }

    void Predefined::MakeBasicVS(ID3D11Device& device)
    {
        basicVS_ = VertexShader::FromByteCode(device, BasicVertexShader);
    }

    void Predefined::MakeBasicPS(ID3D11Device& device)
    {
        basicPS_ = PixelShader::FromByteCode(device, BasicPixelShader);
    }

    void Predefined::MakeStencilStates(ID3D11Device& device)
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

            TryHR(device.CreateDepthStencilState(&noDoubleBlendDesc, noDoubleBlending_.GetAddressOf()));
        }
    }

    void Predefined::MakeBlendingStates(ID3D11Device& device)
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

    void Predefined::MakeRasterizerStates(ID3D11Device& device)
    {
        {
            D3D11_RASTERIZER_DESC cullClockwiseDesc{};
            cullClockwiseDesc.FillMode = D3D11_FILL_SOLID;
            cullClockwiseDesc.CullMode = D3D11_CULL_BACK;
            cullClockwiseDesc.FrontCounterClockwise = true;
            cullClockwiseDesc.DepthClipEnable = true;
            TryHR(device.CreateRasterizerState(&cullClockwiseDesc, cullClockWise_.GetAddressOf()));
        }
    }

    void UpdateAndDraw(const BasicDrawContext& drawContext, const BasicObject& object)
    {
        auto& context = drawContext.Context;
        auto& camera = drawContext.Camera;
        auto& lights = drawContext.Lights;
        auto& renderable = object.Renderable;
        auto& material = object.Material;
        auto& transform = object.Transform;
        //update constant buffers
        const auto view = camera.GetView();
        const auto proj = camera.GetProjection();
        const auto world = object.GetWorld();
        UpdateCb(context, renderable.BasicCb, cb::BasicCb{
            world * view * proj,
            world,
            DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(nullptr, world))
        });
        UpdateCb(context, renderable.PerObjectCb, cb::PerObjectLightingInfo{
            cb::Material{material}
        });
        cb::GlobalLightingInfo globalLights;
        globalLights.EyePos = camera.GetEyePos();
        std::transform(lights.begin(), lights.end(), globalLights.Lights, [](const Light& light) { return cb::Light{ light }; });
        globalLights.LightCount = static_cast<std::uint32_t>(lights.size());
        UpdateCb(context, renderable.GlobalLightingCb, globalLights);
        DrawBasic(context, renderable);
    }

    void DrawBasic(ID3D11DeviceContext& context, const BasicRenderable& renderable)
    {
        context.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        SetupGpuMesh(context, renderable.Mesh.Get());
        SetupShader(context, renderable.VS.Get());
        SetupShader(context, renderable.PS.Get());
        const std::array<ID3D11ShaderResourceView*, 1> resources = {
            renderable.Texture.Get()
        };
        context.PSSetShaderResources(0, resources.size(), resources.data());
        const std::array<ID3D11Buffer*, 1> vsCbs = {
            renderable.BasicCb.GpuCb.Get()
        };
        context.VSSetConstantBuffers(0, vsCbs.size(), vsCbs.data());
        const std::array<ID3D11Buffer*, 2> psCbs = {
            renderable.GlobalLightingCb.GpuCb.Get(),
            renderable.PerObjectCb.GpuCb.Get()
        };
        context.PSSetConstantBuffers(0, psCbs.size(), psCbs.data());
        const std::array<ID3D11SamplerState*, 1> samplers = {
            renderable.Sampler.Get()
        };
        context.PSSetSamplers(0, samplers.size(), samplers.data());
        context.DrawIndexed(renderable.Mesh.Get().IndexCount, 0, 0);
    }

    BasicObject::BasicObject(BasicRenderable renderable, Smoothness material, DirectX::XMMATRIX matrix)
        : Renderable{std::move(renderable)},
        Material{std::move(material)}
    {
        DirectX::XMStoreFloat4x4(&Transform, matrix);
    }
}
