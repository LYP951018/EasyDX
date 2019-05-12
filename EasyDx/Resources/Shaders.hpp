#pragma once

#include <d3d11.h>
#include <d3d11shader.h>
#include "../Vertex.hpp"

namespace dx
{
    struct GlobalShaderContext;

    // same value as D3D11_SHADER_VERSION_TYPE
    enum class ShaderKind
    {
        kPixelShader = 0,
        kVertexShader = 1,
        kGeometryShader = 2,
        kHullShader = 3,
        kDomainShader = 4,
        kComputeShader = 5
    };

    inline constexpr std::size_t kShaderKindCount = 6;

    inline constexpr auto kShaderTargetStrings =
        std::array{"ps_5_0", "vs_5_0", "gs_5_0", "hs_5_0", "ds_5_0", "cs_5_0"};

    ShaderKind KindFromReflection(ID3D11ShaderReflection& reflection);

    constexpr const char* ShaderModelFromKind(ShaderKind kind)
    {
        return kShaderTargetStrings[static_cast<std::uint32_t>(kind)];
    }

#define CREATE_SHADER_Decl(shaderName)                  \
    wrl::ComPtr<ID3D11##shaderName> Create##shaderName( \
        ::ID3D11Device& device, gsl::span<const std::byte> byteCode)

    CREATE_SHADER_Decl(VertexShader);
    CREATE_SHADER_Decl(PixelShader);
    CREATE_SHADER_Decl(HullShader);
    CREATE_SHADER_Decl(DomainShader);

    wrl::ComPtr<ID3D11DeviceChild>
    CreateShaderFromByteCode(ID3D11Device& device3D,
                             gsl::span<const std::byte> byteCode,
                             ShaderKind kind);

    wrl::ComPtr<ID3D10Blob> CompileShaderFromFile(const wchar_t* fileName,
                                                  const char* entryPoint,
                                                  const char* shaderModel);

    wrl::ComPtr<ID3D11ShaderReflection>
    ReflectShader(gsl::span<const std::byte> byteCode);

    struct CbFieldInfo
    {
        std::string Name;
        std::uint32_t Size;
        std::uint32_t Start;
    };

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

        std::uint32_t AddInfo(const char* name)
        {
            const std::uint32_t index =
                gsl::narrow<std::uint32_t>(Resources.size());
            Resources.push_back(nullptr);
            Infos.push_back(BoundedResourcesInfo{std::string{name}, index});
            return index;
        }

        void Bind(std::string_view name, wrl::ComPtr<T> resource)
        {
            if (const auto pos =
                    std::find_if(Infos.begin(), Infos.end(),
                                 [&](const BoundedResourcesInfo& info) {
                                     return name == info.Name;
                                 });
                pos != Infos.end())
            {
                wrl::ComPtr<T>& oldResource = Resources[pos - Infos.begin()];
                Ensures(oldResource == nullptr);
                oldResource = std::move(resource);
            }
            else
            {
                const std::uint32_t index = AddInfo(name.data());
                Resources[index] = std::move(resource);
            }
        }
    };

    class ShaderInputs
    {
      public:
        template<typename T>
        void SetField(std::string_view fieldName, const T& value)
        {
            return SetBytes(fieldName, AsBytes(value));
        }

        template<typename T>
        T& BorrowMut(std::string_view name)
        {
            CbFieldInfo& fieldInfo = EnsureFieldExists(name, sizeof(T));
            const auto bytes = BytesFromField(fieldInfo);
            Ensures(sizeof(T) == bytes.size());
            return reinterpret_cast<T&>(*bytes.data());
        }

        void SetBytes(std::string_view fieldName,
                      gsl::span<const std::byte> bytes);

        ShaderInputs& Bind(std::string_view name,
                           wrl::ComPtr<ID3D11ShaderResourceView> resourceView);
        ShaderInputs& Bind(std::string_view name,
                           wrl::ComPtr<ID3D11SamplerState> sampler);
        gsl::span<const Ptr<ID3D11SamplerState>> Samplers() const;
        gsl::span<const Ptr<ID3D11ShaderResourceView>> ResourceViews() const;
        gsl::span<const std::byte> Bytes() const;
        gsl::span<std::byte> Bytes();

      private:
        friend class Shader;

        std::vector<CbFieldInfo>
        CollectFields(std::uint32_t count,
                      ID3D11ShaderReflectionConstantBuffer* cbReflection);
        CbFieldInfo& EnsureFieldExists(std::string_view name, std::size_t size);
        gsl::span<std::byte> BytesFromField(const CbFieldInfo& pInfo);
        gsl::span<const std::byte>
        BytesFromField(const CbFieldInfo& pInfo) const;

        //一般不超过 8 个 constant buffer，线性查找问题不大。
        std::vector<CbFieldInfo> m_fields;
        std::vector<std::byte> m_bytes;
        BoundedResources<ID3D11ShaderResourceView> m_resourceViews;
        BoundedResources<ID3D11SamplerState> m_samplers;
    };

    inline constexpr auto kDefaultEntryName = u8"main";

    class MemoryMappedCso : Noncopyable
    {
      public:
        MemoryMappedCso(const fs::path& path);
        DEFAULT_MOVE(MemoryMappedCso)

        gsl::span<const std::byte> Bytes() const&;

      private:
        FileHandle m_fileHandle;
        MemoryMappedFileHandle m_memoryMappedFile;
        FileMappingViewHandle m_mappedView;
        std::uint64_t m_size;
    };

    struct GpuCbFieldInfo
    {
        std::uint32_t Start;
        std::uint32_t Size;
    };

    struct SharedShaderData
    {
        SharedShaderData(ID3D11Device& device3D,
                         wrl::ComPtr<ID3D11ShaderReflection> reflection_,
            ShaderKind kind,
            gsl::span<const std::byte> byteCode_);

        wrl::ComPtr<ID3D11Buffer> GpuCb;
        wrl::ComPtr<ID3D11ShaderReflection> reflection;
        std::vector<std::byte> CpuBuffer;
        std::unordered_map<std::string, gsl::span<std::byte>> BytesMap;
        BoundedResources<ID3D11ShaderResourceView> ResourceViews;
        BoundedResources<ID3D11SamplerState> Samplers;
        //for vertex shaders only.
        std::vector<std::byte> byteCode;
    };

    class Shader
    {
      public:
        Shader(ShaderKind kind) : m_kind{kind} {}

        Shader(ID3D11Device& device3D, gsl::span<const std::byte> byteCode)
            : Shader{device3D, byteCode, ReflectShader(byteCode)}
        {}

        Shader(Shader&& rhs) noexcept = default;
        Shader& operator=(Shader&&) noexcept = default;
        Shader(const Shader&) = default;
        Shader& operator=(const Shader&) = default;

        static Shader FromSourceFile(ID3D11Device& device3D,
                                     const wchar_t* path, ShaderKind shaderKind,
                                     const char* entryName = kDefaultEntryName)
        {
            return Shader{device3D, CompileShaderFromFile(
                                        path, entryName,
                                        ShaderModelFromKind(shaderKind))};
        }

        static Shader FromSourceFile(ID3D11Device& device3D,
                                     const fs::path& path,
                                     ShaderKind shaderKind,
                                     const char* entryName = kDefaultEntryName)
        {
            return FromSourceFile(device3D, path.c_str(), shaderKind,
                                  entryName);
        }

        static Shader FromCompiledCso(ID3D11Device& device3D,
                                      const fs::path& path);

        // ShaderKind Kind() const;

        void Apply(const ShaderInputs& inputs) const;
        void Apply(const GlobalShaderContext& shaderContext) const;
        void Flush(ID3D11DeviceContext& context3D) const;
        void Setup(ID3D11DeviceContext& context3D) const;
        void SetBytes(std::string_view fieldName,
                      gsl::span<const std::byte> bytes) const;
        ShaderKind GetKind() const { return m_kind; }
        gsl::span<const std::byte> GetByteCode() const;
        const wrl::ComPtr<ID3D11ShaderReflection>& GetReflection() const
        {
            return m_sharedData->reflection;
        }

        template<typename T>
        void SetField(std::string_view fieldName, const T& value) const
        {
            SetBytes(fieldName, gsl::as_bytes(SingleAsSpan(value)));
        }

        void Bind(std::string_view name,
                  wrl::ComPtr<ID3D11ShaderResourceView> resourceView) const;
        void Bind(std::string_view name,
                  wrl::ComPtr<ID3D11SamplerState> sampler) const;

        explicit operator bool() const { return m_shaderObject != nullptr; }

      private:
        Shader(ID3D11Device& device3D, wrl::ComPtr<ID3DBlob> byteCode)
            : Shader{device3D, AsSpan(Ref(byteCode))}
        {}

        Shader(ID3D11Device& device3D, gsl::span<const std::byte> byteCode,
               wrl::ComPtr<ID3D11ShaderReflection> reflection);

        ShaderKind m_kind;
        std::shared_ptr<SharedShaderData> m_sharedData;
        wrl::ComPtr<ID3D11DeviceChild> m_shaderObject;
    };

    void SetupShader(ID3D11DeviceContext& context3D, const Shader& shader);

    using ShaderArray = std::array<Shader, kShaderKindCount>;

    struct ShaderCollection : private ShaderArray
    {
      private:
        using BaseType = ShaderArray;

      public:
        ShaderCollection(ShaderArray shaders)
            : BaseType{std::move(shaders)}, m_mask{MaskFromVertexShader()}
        {}

        using BaseType::begin;
        using BaseType::data;
        using BaseType::end;
        using BaseType::size;

        VSSemantics GetMask() const { return m_mask; }

        const Shader& operator[](ShaderKind kind) const noexcept
        {
            return static_cast<const BaseType&>(
                *this)[static_cast<std::size_t>(kind)];
        }

        const Shader& GetVertexShader() const
        {
            return (*this)[ShaderKind::kVertexShader];
        }

      private:
        VSSemantics MaskFromVertexShader();
        VSSemantics m_mask;
    };

    class ShadersBuilder
    {
      public:
        ShadersBuilder()
            : m_shaders{Shader{ShaderKind::kPixelShader},
                        Shader{ShaderKind::kVertexShader},
                        Shader{ShaderKind::kGeometryShader},
                        Shader{ShaderKind::kHullShader},
                        Shader{ShaderKind::kDomainShader},
                        Shader{ShaderKind::kComputeShader}}
        {}

        ShadersBuilder& WithVS(Shader vs)
        {
            return Set(ShaderKind::kVertexShader, std::move(vs));
        }

        ShadersBuilder& WithPS(Shader ps)
        {
            return Set(ShaderKind::kPixelShader, std::move(ps));
        }

        ShaderCollection Build() const;

      private:
        ShadersBuilder& Set(ShaderKind kind, Shader shader);

        ShaderArray m_shaders;
    };

    struct Pass;
    struct PassWithShaderInputs;

    ShaderCollection MakeShaderCollection(Shader vertexShader,
                                          Shader pixelShader);
    void SetupShaders(ID3D11DeviceContext& context3D,
                      const ShaderCollection& shaders);
    void FillUpShaders(ID3D11DeviceContext& context3D,
                       const PassWithShaderInputs& passWithInputs,
                       const DirectX::XMMATRIX& world,
                       const ShaderInputs* additionalInput,
                       const GlobalShaderContext& shaderContext);

    struct Shaders
    {
      public:
        static void Add(std::string_view name, Shader shader);
        static std::optional<Shader> Get(std::string_view name);

        static void Setup();
        static void LoadDefaultShaders(ID3D11Device& device3D);

#define DEF_SHADER_NAME(name) \
    inline static constexpr auto CONCAT(k, name) = CONCAT(u8, #name)
        DEF_SHADER_NAME(BasicLighting);
        DEF_SHADER_NAME(PosNormalTexTransform);
        DEF_SHADER_NAME(QuadVS);
        DEF_SHADER_NAME(QuadPS);
        DEF_SHADER_NAME(PosVS);
        DEF_SHADER_NAME(PosNormalTexVS);
        DEF_SHADER_NAME(PosNormTanTexVS);
        DEF_SHADER_NAME(DefaultShadowCasterVS);
        DEF_SHADER_NAME(DefaultShadowCasterPS);
#undef DEF_SHADER_NAME

      private:
        Shaders();
        friend class ShaderCollection;

        std::unordered_map<std::string, Shader> m_shaders;
        std::unordered_map<std::string_view, VSSemantics>
            m_semanticsNameToMaskMap;
    };
} // namespace dx
