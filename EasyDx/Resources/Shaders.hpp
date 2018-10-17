#pragma once

#include <d3d11.h>
#include <d3d11shader.h>
#include "../Vertex.hpp"

namespace dx
{
#define CREATE_SHADER_Decl(shaderName)                                         \
    wrl::ComPtr<ID3D11##shaderName> Create##shaderName(::ID3D11Device& device, \
                                                       gsl::span<const std::byte> byteCode)

    CREATE_SHADER_Decl(VertexShader);
    CREATE_SHADER_Decl(PixelShader);
    CREATE_SHADER_Decl(HullShader);
    CREATE_SHADER_Decl(DomainShader);

    template<typename T>
    wrl::ComPtr<T> CreateShaderFromByteCode(ID3D11Device& device3D,
                                            gsl::span<const std::byte> byteCode)
    {
        if constexpr (std::is_same_v<T, ID3D11VertexShader>)
            return CreateVertexShader(device3D, byteCode);
        else if constexpr (std::is_same_v<T, ID3D11PixelShader>)
            return CreatePixelShader(device3D, byteCode);
        else if constexpr (std::is_same_v<T, ID3D11HullShader>)
            return CreateHullShader(device3D, byteCode);
        else if constexpr (std::is_same_v<T, ID3D11DomainShader>)
            return CreateDomainShader(device3D, byteCode);
        else
            static_assert(always_false<T>::value);
    }

    wrl::ComPtr<ID3D10Blob> CompileShaderFromFile(const wchar_t* fileName, const char* entryPoint,
                                                  const char* shaderModel);

    wrl::ComPtr<ID3D11ShaderReflection> ReflectShader(gsl::span<const std::byte> byteCode);

    struct CbFieldInfo
    {
        std::string Name;
        std::uint32_t Size;
        std::uint32_t Start;
    };

    struct ConstantBufferInfo
    {
      public:
        template<typename T>
        void Set(std::string_view fieldName, const T& value)
        {
            SetBytes(fieldName, AsBytes(value));
        }

        void SetBytes(std::string_view fieldName, gsl::span<const std::byte> bytes);
        gsl::span<const std::byte> GetBytes(std::string_view name) const;
        gsl::span<std::byte> GetBytesMut(std::string_view name);

        template<typename T>
        T& BorrowMut(std::string_view name)
        {
            const auto bytes = GetBytesMut(name);
            Ensures(sizeof(T) == bytes.size());
            return reinterpret_cast<T&>(*bytes.data());
        }

        std::string_view Name() const;
        gsl::span<const std::byte> Bytes() const;
        gsl::span<std::byte> Bytes();

      private:
        friend class ShaderInputs;

        ConstantBufferInfo(std::string name, std::uint32_t size,
                           std::vector<CbFieldInfo> fieldInfos, std::uint32_t cbIndex);
        gsl::span<std::byte> BytesFromField(const CbFieldInfo& pInfo);
        gsl::span<const std::byte> BytesFromField(const CbFieldInfo& pInfo) const;
        const CbFieldInfo* FindByName(std::string_view name) const;
        bool IsDirty() const { return m_isDirty; }

        const std::string m_name;
        std::vector<std::byte> m_bytes;
        const std::vector<CbFieldInfo> m_fields;
        const std::uint32_t m_cbIndex;
        mutable bool m_isDirty;
    };

    class ShaderInputs
    {
      public:
        ShaderInputs(ID3D11Device& device3D, wrl::ComPtr<ID3D11ShaderReflection> reflection);

        template<typename T>
        ShaderInputs& Set(std::string_view cbName, std::string_view fieldName, const T& value)
        {
            return SetBytes(cbName, fieldName, AsBytes(value));
        }

        ShaderInputs& SetBytes(std::string_view cbName, std::string_view fieldName,
                      gsl::span<const std::byte> bytes);

        ShaderInputs& Bind(std::string_view name,
                          wrl::ComPtr<ID3D11ShaderResourceView> resourceView);
        ShaderInputs& Bind(std::string_view name, wrl::ComPtr<ID3D11SamplerState> sampler);
        ConstantBufferInfo* GetCbInfo(std::string_view name);
        ConstantBufferInfo* GetCbInfo(std::uint32_t index);
        gsl::span<const Ptr<ID3D11Buffer>> Buffers() const;
        gsl::span<const Ptr<ID3D11SamplerState>> Samplers() const;
        gsl::span<const Ptr<ID3D11ShaderResourceView>> ResourceViews() const;
        void Flush(ID3D11DeviceContext& context3D) const;
        void Clear();

      private:
        struct BoundedResourcesInfo
        {
            std::string Name;
            std::uint32_t Index;
        };

        template<typename T>
        struct BoundedResources
        {
            std::vector<wrl::ComPtr<T>> Resources;
            std::vector<BoundedResourcesInfo> Infos;

            void AddInfo(const char* name)
            {
                const std::uint32_t index = gsl::narrow<std::uint32_t>(Resources.size());
                Resources.push_back(nullptr);
                Infos.push_back(BoundedResourcesInfo{std::string{name}, index});
            }

            void Bind(std::string_view name, wrl::ComPtr<T> resource)
            {
                if (const auto pos = std::find_if(
                        Infos.begin(), Infos.end(),
                        [&](const BoundedResourcesInfo& info) { return name == info.Name; });
                    pos != Infos.end())
                {
                    wrl::ComPtr<T>& oldResource = Resources[pos - Infos.begin()];
                    Ensures(oldResource == nullptr);
                    oldResource = std::move(resource);
                }
                else
                {
                    // TODO
                    assert(false);
                }
            }
        };

        std::vector<CbFieldInfo> CollectFields(std::uint32_t count,
                                               ID3D11ShaderReflectionConstantBuffer* cbReflection);
        bool AnyCbDirty() const;

        // constant buffers
        std::vector<wrl::ComPtr<ID3D11Buffer>> m_cbs;
        //一般不超过 8 个 constant buffer，线性查找问题不大。
        std::vector<ConstantBufferInfo> m_cbInfos;
        BoundedResources<ID3D11ShaderResourceView> m_resourceViews;
        BoundedResources<ID3D11SamplerState> m_samplers;
    };

    template<typename T>
    struct ShaderWithInputs
    {
        ShaderWithInputs(ID3D11Device& device3D, gsl::span<const std::byte> byteCode)
            : Shader{CreateShaderFromByteCode<T>(device3D, byteCode)}, Inputs{
                                                                           device3D,
                                                                           ReflectShader(byteCode)}
        {}

        wrl::ComPtr<T> Shader;
        ShaderInputs Inputs;
    };

    using VertexShader = ShaderWithInputs<ID3D11VertexShader>;
    using PixelShader = ShaderWithInputs<ID3D11PixelShader>;
    using HullShader = ShaderWithInputs<ID3D11HullShader>;
    using DomainShader = ShaderWithInputs<ID3D11DomainShader>;

    struct ShaderCollection
    {
        ShaderCollection(VertexShader vs, PixelShader ps);
        ShaderCollection(VertexShader vs, PixelShader ps, HullShader hs, DomainShader ds);

        VertexShader VertexShader_;
        PixelShader PixelShader_;
        std::unique_ptr<HullShader> HullShader_;
        std::unique_ptr<DomainShader> DomainShader_;
    };

    void Bind(ID3D11DeviceContext& context3D, const VertexShader& shader);
    void Bind(ID3D11DeviceContext& context3D, const PixelShader& shader);

    void SetupShaders(ID3D11DeviceContext& context3D, const ShaderCollection& shaders);
} // namespace dx
