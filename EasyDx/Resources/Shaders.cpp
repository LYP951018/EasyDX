#include "../pch.hpp"
#include <gsl/gsl_assert>
#include <d3d11.h>
#include <D3Dcompiler.h>
#include "Shaders.hpp"
#include "Buffers.hpp"
#include "../Material.hpp"
#include "../GlobalShaderContext.hpp"
#include "../ShaderCbKeyDef.hpp"

namespace dx
{
#define CREATE_SHADER_DEFINE(shaderName)                                  \
    wrl::ComPtr<ID3D11##shaderName> Create##shaderName(                   \
        ::ID3D11Device& device, gsl::span<const std::byte> byteCode)      \
    {                                                                     \
        wrl::ComPtr<ID3D11##shaderName> shader;                           \
        TryHR(device.Create##shaderName(byteCode.data(), byteCode.size(), \
                                        nullptr, shader.GetAddressOf())); \
        return std::move(shader);                                         \
    }

    CREATE_SHADER_DEFINE(VertexShader)
    CREATE_SHADER_DEFINE(PixelShader)
    CREATE_SHADER_DEFINE(HullShader)
    CREATE_SHADER_DEFINE(DomainShader)

    wrl::ComPtr<ID3D11DeviceChild>
    CreateShaderFromByteCode(ID3D11Device& device3D,
                             gsl::span<const std::byte> byteCode,
                             ShaderKind kind)
    {
        switch (kind)
        {
            case ShaderKind::kPixelShader:
                return CreatePixelShader(device3D, byteCode);
            case ShaderKind::kVertexShader:
                return CreateVertexShader(device3D, byteCode);
            case ShaderKind::kGeometryShader:
                assert(false);
                break;
            case ShaderKind::kHullShader:
                return CreateHullShader(device3D, byteCode);
            case ShaderKind::kDomainShader:
                return CreateDomainShader(device3D, byteCode);
            case ShaderKind::kComputeShader:
                assert(false);
                break;
            default:
                assert(false);
                break;
        }
    }

    wrl::ComPtr<ID3D10Blob> CompileShaderFromFile(const wchar_t* fileName,
                                                  const char* entryPoint,
                                                  const char* shaderModel)
    {
        std::uint32_t compileFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
        compileFlags |= D3DCOMPILE_DEBUG;
        compileFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
        wrl::ComPtr<ID3DBlob> shaderBlob;
        wrl::ComPtr<ID3DBlob> errorBlob;
        const auto hr = D3DCompileFromFile(
            fileName, nullptr, nullptr, entryPoint, shaderModel, compileFlags,
            {}, shaderBlob.GetAddressOf(), errorBlob.GetAddressOf());
        if (FAILED(hr))
        {
            const auto errorMessage =
                "Compile error in shader " + ws2s(fileName) + ", " +
                static_cast<const char*>(errorBlob->GetBufferPointer());
            throw std::runtime_error{errorMessage};
        }
        return shaderBlob;
    }

    void ShaderInputs::SetBytes(std::string_view fieldName,
                                gsl::span<const std::byte> bytes)
    {
        CbFieldInfo& fieldInfo = EnsureFieldExists(fieldName, bytes.size());
        const gsl::span<std::byte> orignalBytes = BytesFromField(fieldInfo);
        gsl::copy(bytes, orignalBytes);
    }

    ShaderInputs&
    ShaderInputs::Bind(std::string_view name,
                       wrl::ComPtr<ID3D11ShaderResourceView> resourceView)
    {
        m_resourceViews.Bind(name, std::move(resourceView));
        return *this;
    }

    ShaderInputs& ShaderInputs::Bind(std::string_view name,
                                     wrl::ComPtr<ID3D11SamplerState> sampler)
    {
        m_samplers.Bind(name, std::move(sampler));
        return *this;
    }

    gsl::span<const Ptr<ID3D11SamplerState>> ShaderInputs::Samplers() const
    {
        return ComPtrsCast(gsl::make_span(m_samplers.Resources));
    }

    gsl::span<const Ptr<ID3D11ShaderResourceView>>
    ShaderInputs::ResourceViews() const
    {
        return ComPtrsCast(gsl::make_span(m_resourceViews.Resources));
    }

    gsl::span<const std::byte> ShaderInputs::Bytes() const
    {
        return gsl::make_span(m_bytes);
    }

    gsl::span<std::byte> ShaderInputs::Bytes()
    {
        return gsl::make_span(m_bytes);
    }

    std::vector<CbFieldInfo> ShaderInputs::CollectFields(
        std::uint32_t count, ID3D11ShaderReflectionConstantBuffer* cbReflection)
    {
        std::vector<CbFieldInfo> fields;
        D3D11_SHADER_VARIABLE_DESC desc;
        for (std::uint32_t i = 0; i < count; ++i)
        {
            const auto varReflection = cbReflection->GetVariableByIndex(i);
            TryHR(varReflection->GetDesc(&desc));
            fields.push_back(CbFieldInfo{std::string{desc.Name}, desc.Size,
                                         desc.StartOffset});
        }
        return fields;
    }

    CbFieldInfo& ShaderInputs::EnsureFieldExists(std::string_view name,
                                                 std::size_t size)
    {
        if (const auto it = std::find_if(
                m_fields.begin(), m_fields.end(),
                [&](const CbFieldInfo& info) { return info.Name == name; });
            it != m_fields.end())
        {
            return *it;
        }
        else
        {
            const std::size_t newStart = m_bytes.size();
            m_bytes.resize(newStart + size);
            return m_fields.emplace_back(
                CbFieldInfo{std::string{name}, gsl::narrow<std::uint32_t>(size),
                            gsl::narrow<std::uint32_t>(newStart)});
        }
    }

    gsl::span<std::byte> ShaderInputs::BytesFromField(const CbFieldInfo& pInfo)
    {
        return Bytes().subspan(pInfo.Start, pInfo.Size);
    }

    gsl::span<const std::byte>
    ShaderInputs::BytesFromField(const CbFieldInfo& pInfo) const
    {
        return Bytes().subspan(pInfo.Start, pInfo.Size);
    }

    wrl::ComPtr<ID3D11ShaderReflection>
    ReflectShader(gsl::span<const std::byte> byteCode)
    {
        void* reflection;
        TryHR(D3DReflect(byteCode.data(), byteCode.size(),
                         IID_ID3D11ShaderReflection, &reflection));
        wrl::ComPtr<ID3D11ShaderReflection> result;
        result.Attach(static_cast<ID3D11ShaderReflection*>(reflection));
        return result;
    }

    void SetupShader(ID3D11DeviceContext& context3D, const Shader& shader)
    {
        shader.Setup(context3D);
    }

    ShaderCollection MakeShaderCollection(Shader vertexShader,
                                          Shader pixelShader)
    {
        return ShadersBuilder{}
            .WithVS(std::move(vertexShader))
            .WithPS(std::move(pixelShader))
            .Build();
    }

    Shader Shader::FromCompiledCso(ID3D11Device& device3D,
                                   const fs::path& csoPath)
    {
        const auto mappedCso = MemoryMappedCso{csoPath};
        return Shader{device3D, mappedCso.Bytes()};
    }

    // ShaderKind Shader::Kind() const { return m_sharedData->Kind; }

    void Shader::Apply(const ShaderInputs& inputs) const
    {
        for (const CbFieldInfo& fieldInfo : inputs.m_fields)
        {
            SetBytes(fieldInfo.Name, inputs.BytesFromField(fieldInfo));
        }

        // FIXME!!!
        m_sharedData->ResourceViews = inputs.m_resourceViews;
        m_sharedData->Samplers = inputs.m_samplers;
    }

    void Shader::Apply(const GlobalShaderContext& shaderContext) const
    {
        shaderContext.Apply(m_sharedData->BytesMap);
    }

    void Shader::Flush(ID3D11DeviceContext& context3D) const
    {
        if (m_sharedData->GpuCb)
        {
            UpdateWithDiscard(
                context3D, dx::Ref(m_sharedData->GpuCb),
                gsl::span<const std::byte>(m_sharedData->CpuBuffer));
        }
    }

#define BIND_WITH_PREFIX(prefix, type)                                      \
    {                                                                       \
        wrl::ComPtr<type> shader;                                           \
        if (m_shaderObject)                                                 \
        {                                                                   \
            const auto samplers = dx::ComPtrsCast(                          \
                gsl::make_span(m_sharedData->Samplers.Resources));          \
            context3D.CONCAT(prefix, SetSamplers)(                          \
                0, gsl::narrow<std::uint32_t>(samplers.size()),             \
                samplers.data());                                           \
            const auto views = dx::ComPtrsCast(                             \
                gsl::make_span(m_sharedData->ResourceViews.Resources));     \
            context3D.CONCAT(prefix, SetShaderResources)(                   \
                0, gsl::narrow<std::uint32_t>(views.size()), views.data()); \
            const auto cbs =                                                \
                dx::ComPtrsCast(dx::SingleAsSpan(m_sharedData->GpuCb));     \
            context3D.CONCAT(prefix, SetConstantBuffers)(                   \
                0, gsl::narrow<std::uint32_t>(cbs.size()), cbs.data());     \
            TryHR(m_shaderObject.As(&shader));                              \
        }                                                                   \
        context3D.CONCAT(prefix, SetShader)(shader.Get(), nullptr, 0);      \
    }

    void Shader::Setup(ID3D11DeviceContext& context3D) const
    {
        switch (GetKind())
        {
            case ShaderKind::kPixelShader:
                BIND_WITH_PREFIX(PS, ID3D11PixelShader)
                break;
            case ShaderKind::kVertexShader:
                BIND_WITH_PREFIX(VS, ID3D11VertexShader)
                break;
            case ShaderKind::kHullShader:
                BIND_WITH_PREFIX(HS, ID3D11HullShader)
                break;
            case ShaderKind::kDomainShader:
                BIND_WITH_PREFIX(DS, ID3D11DomainShader)
                break;
            case ShaderKind::kGeometryShader:
                BIND_WITH_PREFIX(GS, ID3D11GeometryShader)
                break;
            default:
                assert(false);
                break;
        }
    }

    void Shader::SetBytes(std::string_view fieldName,
                          gsl::span<const std::byte> bytes) const
    {
        const auto& bytesMap = m_sharedData->BytesMap;
        // TODO: 异构查找
        if (const auto it = bytesMap.find(std::string{fieldName});
            it != bytesMap.end())
        {
            gsl::span<std::byte> destSpan = it->second;
            gsl::copy(bytes, destSpan);
        }
    }

    void Shader::Bind(std::string_view name,
                      wrl::ComPtr<ID3D11ShaderResourceView> resourceView) const
    {
        m_sharedData->ResourceViews.Bind(name, std::move(resourceView));
    }

    void Shader::Bind(std::string_view name,
                      wrl::ComPtr<ID3D11SamplerState> sampler) const
    {
        m_sharedData->Samplers.Bind(name, std::move(sampler));
    }

    ShaderKind KindFromReflection(ID3D11ShaderReflection& reflection)
    {
        D3D11_SHADER_DESC desc;
        TryHR(reflection.GetDesc(&desc));
        return static_cast<ShaderKind>(D3D11_SHVER_GET_TYPE(desc.Version));
    }

    Shader::Shader(ID3D11Device& device3D, gsl::span<const std::byte> byteCode,
                   wrl::ComPtr<ID3D11ShaderReflection> reflection)
        : m_sharedData{std::make_shared<SharedShaderData>(device3D,
                                                          reflection)},
          m_kind{KindFromReflection(dx::Ref(reflection))},
          m_shaderObject{
              CreateShaderFromByteCode(device3D, byteCode, GetKind())}
    {}

    SharedShaderData::SharedShaderData(
        ID3D11Device& device3D, wrl::ComPtr<ID3D11ShaderReflection> reflection_)
        : reflection{std::move(reflection_)}
    {
        D3D11_SHADER_DESC shaderDesc;
        TryHR(reflection->GetDesc(&shaderDesc));
        const std::uint32_t cbCount = shaderDesc.ConstantBuffers;
        if (cbCount == 0)
        {
            return;
        }
        Ensures(cbCount == 1);
        D3D11_SHADER_BUFFER_DESC bufferDesc;
        ID3D11ShaderReflectionConstantBuffer* const cbReflection =
            reflection->GetConstantBufferByIndex(0);
        TryHR(cbReflection->GetDesc(&bufferDesc));
        GpuCb = MakeConstantBuffer(device3D, bufferDesc.Size);
        CpuBuffer.resize(static_cast<std::size_t>(bufferDesc.Size));
        D3D11_SHADER_VARIABLE_DESC desc;
        for (std::uint32_t i = 0; i < bufferDesc.Variables; ++i)
        {
            const auto varReflection = cbReflection->GetVariableByIndex(i);
            TryHR(varReflection->GetDesc(&desc));
            BytesMap.insert(std::make_pair(
                desc.Name, gsl::span<std::byte>(CpuBuffer).subspan(
                               desc.StartOffset, desc.Size)));
        }
        const std::uint32_t bindSlotCount = shaderDesc.BoundResources;
        D3D11_SHADER_INPUT_BIND_DESC bindDesc;
        for (std::uint32_t i = 0; i < bindSlotCount; ++i)
        {
            TryHR(reflection->GetResourceBindingDesc(i, &bindDesc));
            switch (bindDesc.Type)
            {
                case D3D_SIT_SAMPLER:
                    Samplers.AddInfo(bindDesc.Name);
                    break;
                case D3D_SIT_TEXTURE:
                    ResourceViews.AddInfo(bindDesc.Name);
                    break;
                    // cbs has been handled
                case D3D_SIT_CBUFFER:
                    continue;
                default:
                    assert(false);
                    break;
            }
        }
    }

    void SetupShaders(ID3D11DeviceContext& context3D,
                      const ShaderCollection& shaders)
    {
        for (const Shader& shader : shaders)
        {
            if (shader)
            {
                shader.Setup(context3D);
            }
        }
    }

    void FillUpShaders(ID3D11DeviceContext& context3D,
                       const PassWithShaderInputs& passWithInputs,
                       const DirectX::XMMATRIX& world,
                       const ShaderInputs* additionalInput,
                       const GlobalShaderContext& shaderContext)
    {
        using namespace DirectX;
        const ShaderInputs& inputs = passWithInputs.inputs;
        const Pass& pass = *passWithInputs.pass;
        for (std::size_t i = 0; i < pass.Shaders.size(); ++i)
        {
            const Shader& shader = pass.Shaders[static_cast<ShaderKind>(i)];
            if (!shader)
                continue;
            shader.Apply(inputs);
            shader.Apply(shaderContext);
            if (additionalInput)
            {
                shader.Apply(*additionalInput);
            }
            shader.SetField(WORLD_MATRIX, world);
            shader.SetField(INV_TRANS_WORLD,
                            XMMatrixInverse(nullptr, XMMatrixTranspose(world)));
            shader.SetField(WORLD_VIEW_PROJ_MATRIX,
                            world * shaderContext.ViewProjMatrix);
            shader.Flush(context3D);
        }
    }

    MemoryMappedCso::MemoryMappedCso(const fs::path& path)
        : m_fileHandle{OpenExistingFile(path, FileAccessMode::Read,
                                        FileShareMode::Read)},
          m_memoryMappedFile{
              OpenWin32WithCheck<MemoryMappedFileHandle>(::CreateFileMappingW(
                  m_fileHandle.Get(), nullptr, PAGE_READONLY, 0, 0, nullptr))},
          m_mappedView{
              OpenWin32WithCheck<FileMappingViewHandle>(::MapViewOfFile(
                  m_memoryMappedFile.Get(), FILE_MAP_READ, 0, 0, 0))}
    {
        // FIXME: fileSize may be larger than 2GB, this code would not work
        // with 32bit.
        LARGE_INTEGER fileSize{};
        if (::GetFileSizeEx(m_fileHandle.Get(), &fileSize) == FALSE)
        {
            ThrowWin32();
        }
        m_size = fileSize.QuadPart;
    }

    gsl::span<const std::byte> MemoryMappedCso::Bytes() const&
    {
        return gsl::span<const std::byte>{
            static_cast<const std::byte*>(m_mappedView.Get()),
            gsl::narrow<std::ptrdiff_t>(m_size)};
    }

    std::unique_ptr<Shaders> g_shaders;

    void Shaders::Add(std::string_view name, Shader shader)
    {
        g_shaders->m_shaders.insert(std::make_pair(name, std::move(shader)));
    }

    std::optional<Shader> Shaders::Get(std::string_view name)
    {
        const auto& shaders = g_shaders->m_shaders;
        // TODO: 异构查找
        if (const auto it = shaders.find(std::string{name});
            it != shaders.end())
        {
            return it->second;
        }
        return std::nullopt;
    }

    void Shaders::Setup() { g_shaders = std::make_unique<Shaders>(Shaders{}); }

    ShaderCollection ShadersBuilder::Build() const
    {
        return ShaderCollection{std::move(m_shaders)};
    }

    ShadersBuilder& ShadersBuilder::Set(ShaderKind kind, Shader shader)
    {
        assert(shader.GetKind() == kind);
        m_shaders[static_cast<std::size_t>(kind)] = std::move(shader);
        return *this;
    }

    VSSemantics ShaderCollection::MaskFromVertexShader()
    {
        const Shader& vs = GetVertexShader();
        ID3D11ShaderReflection* const reflection = vs.GetReflection().Get();
        D3D11_SHADER_DESC shaderDesc;
        TryHR(reflection->GetDesc(&shaderDesc));
        VSSemantics mask = VSSemantics::kNone;
        for (std::uint32_t i = 0; i < shaderDesc.InputParameters; ++i)
        {
            D3D11_SIGNATURE_PARAMETER_DESC paramDesc;
            TryHR(reflection->GetInputParameterDesc(i, &paramDesc));
            const std::string_view semantics = paramDesc.SemanticName;
            // FIXME: semantics not found
            mask |= g_shaders->m_semanticsNameToMaskMap.find(semantics)->second;
        }
        return mask;
    }

    Shaders::Shaders()
        : m_semanticsNameToMaskMap{{"POSITION", VSSemantics::kPosition},
                                   {"TANGENT", VSSemantics::kTangent},
                                   {"TEXCOORD", VSSemantics::kTexCoord},
                                   {"NORMAL", VSSemantics::kNormal},
                                   {"COLOR", VSSemantics::kColor}}
    {
        // m_semanticsNameToMaskMap.insert(std::make_pair("POSITION",
        // VSSemantics::kPosition));
    }
} // namespace dx
