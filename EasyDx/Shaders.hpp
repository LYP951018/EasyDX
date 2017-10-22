#pragma once

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

    struct VertexShaderView
    {
        ID3D11VertexShader* Shader;
        ID3D11InputLayout* Layout;
    };

    struct VertexShader
    {
    public:
        VertexShader() = default;

        static VertexShader CompileFromFile(ID3D11Device& device,
            const fs::path& filePath,
            const char* entryName,
            gsl::span<const D3D11_INPUT_ELEMENT_DESC> layoutDesc);

        static VertexShader FromByteCode(ID3D11Device& device,
            gsl::span<const std::byte> byteCode,
            gsl::span<const D3D11_INPUT_ELEMENT_DESC> layoutDesc);

        //TODO: Why not constructor?
        template<std::size_t N>
        static VertexShader FromByteCode(ID3D11Device& device,
            const unsigned char(&byteCode)[N], gsl::span<const D3D11_INPUT_ELEMENT_DESC> layoutDesc)
        {
            const auto p = byteCode;
            return FromByteCode(device, gsl::make_span(reinterpret_cast<const std::byte*>(p),
                static_cast<std::ptrdiff_t>(N)), layoutDesc);
        }

        ID3D11VertexShader& GetShader() const;
        ID3D11InputLayout& GetLayout() const;

        VertexShaderView Get() const noexcept;
        ~VertexShader();

    private:
        VertexShader(wrl::ComPtr<ID3D11VertexShader> shader,
            wrl::ComPtr<ID3D11InputLayout> layout);

        wrl::ComPtr<ID3D11VertexShader> shader_;
        wrl::ComPtr<ID3D11InputLayout> layout_;
    };

#define SHADER_Decl(shaderName) \
    using shaderName##View = ID3D11##shaderName*; \
    struct shaderName { \
    public: shaderName() = default; \
    static shaderName CompileFromFile(ID3D11Device&, const fs::path&, const char*); \
    static shaderName FromByteCode(ID3D11Device&, gsl::span<const std::byte>); \
    template<std::size_t N> \
    static shaderName FromByteCode(ID3D11Device& device,\
        const unsigned char(&byteCode)[N])\
    {\
        const auto p = byteCode;\
        return FromByteCode(device, gsl::make_span(reinterpret_cast<const std::byte*>(p),\
            static_cast<std::ptrdiff_t>(N)));\
    }\
    ID3D11##shaderName& GetShader() const noexcept;\
    shaderName##View Get() const noexcept;\
    ~shaderName();\
    private:\
        shaderName(wrl::ComPtr<ID3D11##shaderName> shader);\
        wrl::ComPtr<ID3D11##shaderName> shader_;\
    };

    SHADER_Decl(PixelShader)
    SHADER_Decl(HullShader)
    SHADER_Decl(DomainShader)
    
    struct ShaderViewGroup
    {
        VertexShaderView VS = {};
        PixelShaderView PS = {};
        HullShaderView HS = {};
        DomainShaderView DS = {};
    };

    void SetShaders(ID3D11DeviceContext& context, const ShaderViewGroup& shaders);

    struct ShaderGroup
    {
    public:
        template<std::size_t VI_, std::size_t PI_, std::size_t HI_, std::size_t DI_>
        ShaderGroup(ID3D11Device& device, const unsigned char(&vbc)[VI_], 
            gsl::span<const D3D11_INPUT_ELEMENT_DESC> layoutDesc,
            const unsigned char(&pbc)[PI_],
            const unsigned char(&hbc)[HI_],
            const unsigned char(&dbc)[DI_])
            : vs_{VertexShader::FromByteCode(device, vbc, layoutDesc)},
            ps_{PixelShader::FromByteCode(device, pbc)},
            hs_{HullShader::FromByteCode(device, hbc)},
            ds_{DomainShader::FromByteCode(device, dbc)}
        {}

        ShaderViewGroup Get() const noexcept
        {
            return {
                vs_.Get(), ps_.Get(), hs_.Get(), ds_.Get()
            };
        }

    private:
        VertexShader vs_;
        PixelShader ps_;
        HullShader hs_;
        DomainShader ds_;
    };

#undef CREATE_SHADER_Decl
#undef SHADER_Decl
}