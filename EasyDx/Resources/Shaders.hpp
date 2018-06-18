#pragma once

#include <d3d11.h>

namespace dx
{
#define CREATE_SHADER_Decl(shaderName) \
    wrl::ComPtr<ID3D11##shaderName> Create##shaderName(::ID3D11Device& device, gsl::span<const std::byte> byteCode)

    CREATE_SHADER_Decl(VertexShader);
    CREATE_SHADER_Decl(PixelShader);
    CREATE_SHADER_Decl(HullShader);
    CREATE_SHADER_Decl(DomainShader);

    wrl::ComPtr<ID3D10Blob> CompileShaderFromFile(
        const wchar_t* fileName,
        const char* entryPoint,
        const char* shaderModel);

#define DEF_SHADER(prefix, shaderName, className, shaderModel) \
    template<typename... ConstantBufferT>\
    struct className\
    {\
    public:\
        static constexpr std::size_t kCbSize = sizeof...(ConstantBufferT);\
        using NativeType = CONCAT(ID3D11, shaderName); \
\
        className() = default;\
\
        /*TODO: allow to pass constant buffer directly */ \
        className(ID3D11Device& device, wrl::ComPtr<ID3D10Blob> byteCode)\
            : shader_{ CONCAT(Create, shaderName)(device, AsSpan(dx::Ref(byteCode)))},\
            gpuCbs_{MakeConstantBuffer<ConstantBufferT>(device)...},\
            byteCode_{std::move(byteCode)}\
        {}\
\
    template<std::size_t N>\
    className(ID3D11Device& device, const unsigned char(&byteCode)[N])\
        : shader_{ CONCAT(Create, shaderName)(device, gsl::make_span(reinterpret_cast<const std::byte*>(byteCode), N)) }, \
        gpuCbs_{ MakeConstantBuffer<ConstantBufferT>(device)... }, \
        byteCode_{ gsl::make_span(reinterpret_cast<const std::byte*>(byteCode), N) }\
    {}\
\
        className(ID3D11Device& device, const fs::path& filePath, const char* entryName)\
            : className{device, CompileShaderFromFile(filePath.c_str(), entryName, shaderModel)}\
        {}\
\
        void UpdateCb(ID3D11DeviceContext& context, const ConstantBufferT&... cpuCbs)\
        {\
            this->UpdateCbImpl(context, std::forward_as_tuple(cpuCbs...), std::make_index_sequence<sizeof...(ConstantBufferT)>{});\
        }\
\
        void SetResources(ID3D11DeviceContext& context, gsl::span<const Ptr<ID3D11ShaderResourceView>> resources, std::uint32_t startSlot = 0) \
{\
        context.CONCAT(prefix, SetShaderResources)(startSlot, gsl::narrow<UINT>(resources.size()), resources.data());\
    }\
        void SetResource(ID3D11DeviceContext& context, Ptr<ID3D11ShaderResourceView> resource, std::uint32_t startSlot = 0) \
{\
        context.CONCAT(prefix, SetShaderResources)(startSlot, 1, &resource);\
    }\
        NativeType* D3DShader() const noexcept { return shader_.Get(); }\
        gsl::span<const Ptr<ID3D11Buffer>, kCbSize> GpuCbs() const noexcept { return ComPtrsCast(gsl::span<const wrl::ComPtr<ID3D11Buffer>, kCbSize>(gpuCbs_)); }\
        gsl::span<const std::byte> ByteCode() const noexcept { \
            switch (byteCode_.index())\
            {\
                case 0:\
                    return AsSpan(Ref(std::get<0>(byteCode_))); \
                case 1:\
                    return std::get<1>(byteCode_); \
            }\
            assert(false);\
    }       \
HOOK \
    private:\
        template<std::size_t... I>\
        void UpdateCbImpl(ID3D11DeviceContext& context, std::tuple<const ConstantBufferT&...> cpuCbs, std::index_sequence<I...>)\
        {\
           (dx::UpdateGpuBuffer(context, Ref(std::get<I>(gpuCbs_)), AsBytes(std::get<I>(cpuCbs))), ...);\
        }\
                \
        wrl::ComPtr<NativeType> shader_;\
        std::array<wrl::ComPtr<ID3D11Buffer>, kCbSize> gpuCbs_;\
        std::variant<wrl::ComPtr<ID3D10Blob>, gsl::span<const std::byte>> byteCode_;  \
    };\
        template<typename... ConstantBufferT> \
        void Bind(ID3D11DeviceContext& context, const className<ConstantBufferT...>& shader)\
        {\
            context.CONCAT(prefix, SetShader)(shader.D3DShader(), {}, {});\
            const auto& gpuCbs = shader.GpuCbs();\
            context.CONCAT(prefix, SetConstantBuffers)(0, gsl::narrow<UINT>(gpuCbs.size()), gpuCbs.data()); \
        }\

#define DEF_SHADER_SIMPLE(prefix, shaderName, shaderModel) DEF_SHADER(prefix, shaderName, shaderName, shaderModel)

#define HOOK
    DEF_SHADER_SIMPLE(HS, HullShader, "hs_5_0")
    DEF_SHADER_SIMPLE(DS, DomainShader, "ds_5_0")
    DEF_SHADER_SIMPLE(VS, VertexShader, "vs_5_0")

//Make clang happy
#undef HOOK

#define HOOK \
void SetSamplers(ID3D11DeviceContext& context3D, gsl::span<const Ptr<ID3D11SamplerState>> samplers, std::uint32_t startSlot = 0) \
{\
context3D.PSSetSamplers(startSlot, gsl::narrow<UINT>(samplers.size()), samplers.data());\
    }\
        void SetSampler(ID3D11DeviceContext& context3D, Ptr<ID3D11SamplerState> sampler, std::uint32_t startSlot = 0) \
    {\
        context3D.PSSetSamplers(startSlot, 1, &sampler); \
    }
        DEF_SHADER_SIMPLE(PS, PixelShader, "ps_5_0")
#undef HOOK
}

#undef CREATE_SHADER_Decl
#undef DEF_SHADER
#undef DEF_SHADER_SIMPLE