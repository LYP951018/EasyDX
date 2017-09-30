#include "pch.hpp"
#include "Predefined.hpp"
#include "Renderable.hpp"
#include "Camera.hpp"
#include <d3d11.h>
#include <DirectXtk/DirectXHelpers.h>

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
        perObjectLightingCbUpdator_ = MakeShared<PerObjectLightingCbUpdator>();
    }

    void Predefined::SetupCamera(const Camera& camera)
    {
        basicCbUpdator_ = MakeShared<BasicCbUpdator>(camera);
    }

    Rc<Renderable> Predefined::GetRenderable(gsl::span<SimpleVertex> vertices, gsl::span<std::uint16_t> indices) const
    {
        auto vs = basicVS_;
        vs.Cbs.push_back(vsCb_.second);
        auto ps = basicPS_;
        ps.Cbs.push_back(psPerObjectLightingCb_.second);
        return MakeComponent<Renderable>(std::move(vs), std::move(ps), SimpleCpuMeshView{ vertices, indices }, std::vector<Rc<IConstantBuffer>>{ vsCb_.first, psPerObjectLightingCb_.first });
    }

    Rc<BasicCbUpdator> Predefined::GetBasicCbUpdator() const
    {
        assert(basicCbUpdator_ != nullptr);
        return basicCbUpdator_;
    }

    void Predefined::MakeWhiteTex(ID3D11Device& device)
    {
        std::uint32_t pixels[kDefaultTexHeight * kDefaultTexWidth];
        std::fill(pixels, pixels + std::size(pixels), static_cast<std::uint32_t>(-1));
        wrl::ComPtr<ID3D11Texture2D> tex;
        CD3D11_TEXTURE2D_DESC desc{ DXGI_FORMAT_R32G32B32A32_FLOAT, kDefaultTexWidth, kDefaultTexHeight };
        D3D11_SUBRESOURCE_DATA resourceData{};
        resourceData.pSysMem = pixels;
        TryHR(device.CreateTexture2D(&desc, &resourceData, tex.GetAddressOf()));
        {
            CD3D11_SAMPLER_DESC desc{ CD3D11_DEFAULT{} };
            //desc.Filter = D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
            device.CreateSamplerState(&desc, defaultSampler_.GetAddressOf());
        }
        white_ = MakeComponent<Texture>(device, std::move(tex), defaultSampler_);
    }

    void Predefined::MakeBasicVS(ID3D11Device& device)
    {
        basicVS_ = MakeVS(device, "BasicVS.hlsl");
        basicVS_.Cbs.push_back(vsCb_.second);
    }

    void Predefined::MakeBasicPS(ID3D11Device& device)
    {
        basicPS_ = MakePS(device, "BasicPS.hlsl");
        basicPS_.Cbs.push_back(psPerObjectLightingCb_.second);
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
            D3D11_BLEND_DESC transparentDesc = { 0 };
            transparentDesc.AlphaToCoverageEnable = false;
            transparentDesc.IndependentBlendEnable = false;

            transparentDesc.RenderTarget[0].BlendEnable = true;
            transparentDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
            transparentDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
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

    VertexShader Predefined::MakeVS(ID3D11Device& device, const char * fileName)
    {
        return VertexShader::CompileFromFile(device, fs::current_path() / "Shaders" / fileName, "main", SimpleVertex::GetLayout());
    }

    PixelShader Predefined::MakePS(ID3D11Device& device, const char* fileName)
    {
        return PixelShader::CompileFromFile(device, fs::current_path() / "Shaders" / fileName, "main");
    }
    
    void BasicCbUpdator::Update(GameObject& object, const UpdateArgs &)
    {
        auto renderable = GetComponent<Renderable>(object);
        //1. update VS cb.
        {
            auto basicCb = std::dynamic_pointer_cast<cb::Basic>(renderable->CpuCbs[0]);
            auto& data = basicCb->Data();
            const auto world = ComputeWorld(object.Transform);
            data.World = world;
            data.WorldInvTranspose = DirectX::XMMatrixInverse({}, DirectX::XMMatrixTranspose(world));
            data.WorldViewProj = world * Camera_.GetView() * Camera_.GetProjection();
        }
        //2. update PS cb.
        {
            auto smoothness = GetComponent<Smoothness>(object);
            auto lightingCb = std::dynamic_pointer_cast<cb::CbPerObjectLightingInfo>(renderable->CpuCbs[1]);
            auto& data = lightingCb->Data();
            data.Smoothness.FromSmoothness(*smoothness);
        }
    }

    void PerObjectLightingCbUpdator::Update(GameObject & object, const UpdateArgs &)
    {
        auto renderable = GetComponent<Renderable>(object);
        auto lightingCb = std::dynamic_pointer_cast<cb::CbPerObjectLightingInfo>(renderable->CpuCbs[1]);
        auto& data = lightingCb->Data();
        auto smoothness = GetComponent<Smoothness>(object);
        data.Smoothness.FromSmoothness(*smoothness);
    }
}
