#pragma once

#include <d3d11.h>

namespace dx
{
#define CREATE_SHADER_Decl(shaderName) \
    wrl::ComPtr<ID3D11##shaderName> Create##shaderName(ID3D11Device& device, gsl::span<const std::byte> byteCode);

    CREATE_SHADER_Decl(VertexShader)
    CREATE_SHADER_Decl(PixelShader)
    CREATE_SHADER_Decl(HullShader)
    CREATE_SHADER_Decl(DomainShader)

    wrl::ComPtr<ID3D10Blob> CompileShaderFromFile(
        const wchar_t* fileName,
        const char* entryPoint,
        const char* shaderModel);

    template<typename ShaderT,
        auto (*CreationFunc) (ID3D11Device& device, gsl::span<const std::byte> byteCode) -> wrl::ComPtr<ShaderT>,
        typename... ConstantBufferT>
    struct Shader
    {
    public:
        using Self = Shader<ShaderT, CreationFunc, ConstantBufferT...>;
        static constexpr std::size_t CbSize = sizeof...(ConstantBufferT);

        Shader() = default;

        Shader(ID3D11Device& device, gsl::span<const std::byte> byteCode)
            : shader_{CreationFunc(device, byteCode)},
            gpuCbs_{MakeConstantBuffer<ConstantBufferT>(device).GpuCb...}
        {}

        //FIXME
        Shader(ID3D11Device& device, const fs::path& filePath, const char* entryName);

        void UpdateCb(ID3D11DeviceContext& context, const ConstantBufferT&... cpuCbs)
        {
            this->UpdateCbImpl(context, std::forward_as_tuple(cpuCbs...), std::make_index_sequence<sizeof...(ConstantBufferT)>{});
        }

        ShaderT* D3DShader() const noexcept { return shader_.Get(); }
        const std::array<ID3D11Buffer*, CbSize>& GpuCbs() const noexcept { return reinterpret_cast<const std::array<ID3D11Buffer*, CbSize>&>(reinterpret_cast<const std::byte&>(gpuCbs_)); }

    private:
        template<std::size_t... I>
        void UpdateCbImpl(ID3D11DeviceContext& context, std::tuple<const ConstantBufferT&...> cpuCbs, std::index_sequence<I...>)
        {
            const auto dummy = [&](ID3D11Buffer& buffer, gsl::span<const std::byte> bytes)
            {
                dx::UpdateConstantBuffer(context, buffer, bytes);
                return 0;
            };
            dx::Swallow(dummy(Ref(std::get<I>(gpuCbs_)), AsBytes(std::get<I>(cpuCbs)))...);
        }

        wrl::ComPtr<ShaderT> shader_;
        std::array<wrl::ComPtr<ID3D11Buffer>, CbSize> gpuCbs_;
    };

    template<typename... ConstantBufferT>
    using PixelShader = Shader<ID3D11PixelShader, CreatePixelShader, ConstantBufferT...>;

    template<typename... ConstantBufferT>
    void SetupShader(ID3D11DeviceContext& context, const PixelShader<ConstantBufferT...>& ps)
    {
        context.PSSetShader(ps.D3DShader(), {}, {});
        const auto& gpuCbs = ps.GpuCbs();
        context.PSSetConstantBuffers(0, gpuCbs.size(), gpuCbs.data());
    }

    template<typename... ConstantBufferT>
    using HullShader = Shader<ID3D11HullShader, CreateHullShader, ConstantBufferT...>;

    template<typename... ConstantBufferT>
    void SetupShader(ID3D11DeviceContext& context, const HullShader<ConstantBufferT...>& hs)
    {
        context.HSSetShader(hs.D3DShader(), {}, {});
        const auto& gpuCbs = hs.GpuCbs();
        context.HSSetConstantBuffers(0, gpuCbs.size(), gpuCbs.data());
    }

    template<typename... ConstantBufferT>
    using DomainShader = Shader<ID3D11DomainShader, CreateDomainShader, ConstantBufferT...>;

    template<typename... ConstantBufferT>
    void SetupShader(ID3D11DeviceContext& context, const DomainShader<ConstantBufferT...>& ds)
    {
        context.DSSetShader(ds.D3DShader(), {}, {});
        const auto& gpuCbs = ds.GpuCbs();
        context.DSSetConstantBuffers(0, gpuCbs.size(), gpuCbs.data());
    }

    template<typename VertexT, typename... ConstantBufferT>
    struct VertexShader : private Shader<ID3D11VertexShader, CreateVertexShader, ConstantBufferT...>
    {
        using Base = Shader<ID3D11VertexShader, CreateVertexShader, ConstantBufferT...>;
        static_assert(is_vertex_v<VertexT>);

    public:
        using Base::UpdateCb;
        using Base::D3DShader;
        using Base::GpuCbs;

        VertexShader() = default;
        
        VertexShader(ID3D11Device& device, gsl::span<const std::byte> byteCode)
            : Base{device, byteCode}
        {
            const auto desc = VertexT::GetDesc();
            device.CreateInputLayout(desc.data(), static_cast<UINT>(desc.size()), byteCode.data(), byteCode.length(), layout_.GetAddressOf());
        }

        ID3D11InputLayout* D3DLayout() const noexcept { return layout_.Get(); }

    private:
        wrl::ComPtr<ID3D11InputLayout> layout_;
    };

    template<typename VertexT, typename... ConstantBufferT>
    void SetupShader(ID3D11DeviceContext& context, const VertexShader<VertexT, ConstantBufferT...>& vs)
    {
        context.VSSetShader(vs.D3DShader(), {}, {});
        const auto& gpuCbs = vs.GpuCbs();
        context.VSSetConstantBuffers(0, gpuCbs.size(), gpuCbs.data());
        context.IASetInputLayout(vs.D3DLayout());
    }

    //BLOCKED:
    /*template<typename... Vcb,
        typename... Pcb,
        typename... Hcb,
        typename... Dcb>
    void SetupShaders(ID3D11DeviceContext& context, const VertexShader<Vcb...>& vc, const PixelShader<Pcb...>& ps, const HullShader<Hcb...>& hs, const DomainShader<Dcb...>& ds)
    {
        SetupShader(context, vc);
        SetupShader(context, ps);
        SetupShader(context, hs);
        SetupShader(context, ds);
    }*/

#undef CREATE_SHADER_Decl
}