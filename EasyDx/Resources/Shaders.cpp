#include "../pch.hpp"
#include "Shaders.hpp"
#include <gsl/gsl_assert>
#include <d3d11.h>
#include <D3Dcompiler.h>
#include "Buffers.hpp"

namespace dx
{
#define CREATE_SHADER_DEFINE(shaderName)                                                    \
    wrl::ComPtr<ID3D11##shaderName> Create##shaderName(::ID3D11Device& device,              \
                                                       gsl::span<const std::byte> byteCode) \
    {                                                                                       \
        wrl::ComPtr<ID3D11##shaderName> shader;                                             \
        TryHR(device.Create##shaderName(byteCode.data(), byteCode.size(), nullptr,          \
                                        shader.GetAddressOf()));                            \
        return std::move(shader);                                                           \
    }

    CREATE_SHADER_DEFINE(VertexShader)
    CREATE_SHADER_DEFINE(PixelShader)
    CREATE_SHADER_DEFINE(HullShader)
    CREATE_SHADER_DEFINE(DomainShader)

    wrl::ComPtr<ID3D10Blob> CompileShaderFromFile(const wchar_t* fileName, const char* entryPoint,
                                                  const char* shaderModel)
    {
        std::uint32_t compileFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
        compileFlags |= D3DCOMPILE_DEBUG;
        compileFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
        wrl::ComPtr<ID3DBlob> shaderBlob;
        wrl::ComPtr<ID3DBlob> errorBlob;
        const auto hr =
            D3DCompileFromFile(fileName, nullptr, nullptr, entryPoint, shaderModel, compileFlags,
                               {}, shaderBlob.GetAddressOf(), errorBlob.GetAddressOf());
        if (FAILED(hr))
        {
            const auto errorMessage = "Compile error in shader " + ws2s(fileName) + ", " +
                                      static_cast<const char*>(errorBlob->GetBufferPointer());
            throw std::runtime_error{errorMessage};
        }
        return shaderBlob;
    }

    ShaderInputs::ShaderInputs(ID3D11Device& device3D,
                               wrl::ComPtr<ID3D11ShaderReflection> reflection)
    {
        D3D11_SHADER_DESC desc;
        TryHR(reflection->GetDesc(&desc));
        const std::uint32_t cbCount = desc.ConstantBuffers;
        D3D11_SHADER_BUFFER_DESC bufferDesc;
        for (std::uint32_t i = 0; i < cbCount; ++i)
        {
            const auto cbReflection = reflection->GetConstantBufferByIndex(i);
            TryHR(cbReflection->GetDesc(&bufferDesc));
            m_cbs.push_back(MakeConstantBuffer(device3D, bufferDesc.Size));
            m_cbInfos.push_back(
                ConstantBufferInfo{std::string{bufferDesc.Name}, bufferDesc.Size,
                                   CollectFields(bufferDesc.Variables, cbReflection), i});
        }
        const std::uint32_t bindSlotCount = desc.BoundResources;
        D3D11_SHADER_INPUT_BIND_DESC bindDesc{};
        for (std::uint32_t i = 0; i < bindSlotCount; ++i)
        {
            TryHR(reflection->GetResourceBindingDesc(i, &bindDesc));
            switch (bindDesc.Type)
            {
            case D3D_SIT_SAMPLER:
                m_samplers.AddInfo(bindDesc.Name);
                break;
            case D3D_SIT_TEXTURE:
                m_resourceViews.AddInfo(bindDesc.Name);
                break;
            // cbs has been handled
            case D3D_SIT_CBUFFER:
                continue;
            default:
                Ensures(false);
                break;
            }
        }
    }

    ShaderInputs& ShaderInputs::SetBytes(std::string_view cbName, std::string_view fieldName,
                                         gsl::span<const std::byte> bytes)
    {
        if (const auto pInfo = GetCbInfo(cbName); pInfo != nullptr)
        {
            pInfo->SetBytes(fieldName, bytes);
        }
        else
        {
            // TODO: throw something
            assert(false);
        }
        return *this;
    }

    ShaderInputs& ShaderInputs::Bind(std::string_view name,
                                     wrl::ComPtr<ID3D11ShaderResourceView> resourceView)
    {
        m_resourceViews.Bind(name, std::move(resourceView));
        return *this;
    }

    ShaderInputs& ShaderInputs::Bind(std::string_view name, wrl::ComPtr<ID3D11SamplerState> sampler)
    {
        m_samplers.Bind(name, std::move(sampler));
        return *this;
    }

    ConstantBufferInfo* ShaderInputs::GetCbInfo(std::string_view name)
    {
        if (const auto it =
                std::find_if(m_cbInfos.begin(), m_cbInfos.end(),
                             [&](const ConstantBufferInfo& info) { return info.Name() == name; });
            it != m_cbInfos.end())
        {
            return &*it;
        }
        else
        {
            return nullptr;
        }
    }

    ConstantBufferInfo* ShaderInputs::GetCbInfo(std::uint32_t index)
    {
        if (index < m_cbInfos.size())
        {
            return &m_cbInfos[index];
        }
        else
        {
            return nullptr;
        }
    }

    gsl::span<ID3D11Buffer* const> ShaderInputs::Buffers() const
    {
        Ensures(!AnyCbDirty());
        return ComPtrsCast(gsl::make_span(m_cbs));
    }

    gsl::span<const Ptr<ID3D11SamplerState>> ShaderInputs::Samplers() const
    {
        return ComPtrsCast(gsl::make_span(m_samplers.Resources));
    }

    gsl::span<const Ptr<ID3D11ShaderResourceView>> ShaderInputs::ResourceViews() const
    {
        return ComPtrsCast(gsl::make_span(m_resourceViews.Resources));
    }

    void ShaderInputs::Flush(ID3D11DeviceContext& context3D) const
    {
        for (const auto& cbInfo : m_cbInfos)
        {
            /*if (!cbInfo.m_isDirty)
                continue;*/
            cbInfo.m_isDirty = false;
            auto& gpuBuffer = Ref(m_cbs[cbInfo.m_cbIndex]);
            UpdateWithDiscard(context3D, gpuBuffer, cbInfo.Bytes());
        }
    }

    std::vector<CbFieldInfo>
    ShaderInputs::CollectFields(std::uint32_t count,
                                ID3D11ShaderReflectionConstantBuffer* cbReflection)
    {
        std::vector<CbFieldInfo> fields;
        D3D11_SHADER_VARIABLE_DESC desc;
        for (std::uint32_t i = 0; i < count; ++i)
        {
            const auto varReflection = cbReflection->GetVariableByIndex(i);
            TryHR(varReflection->GetDesc(&desc));
            fields.push_back(CbFieldInfo{std::string{desc.Name}, desc.Size, desc.StartOffset});
        }
        return fields;
    }

    bool ShaderInputs::AnyCbDirty() const
    {
        return std::any_of(m_cbInfos.begin(), m_cbInfos.end(),
                           [](const ConstantBufferInfo& info) { return info.IsDirty(); });
    }

    void ConstantBufferInfo::SetBytes(std::string_view fieldName, gsl::span<const std::byte> bytes)
    {
        if (const auto it =
                std::find_if(m_fields.begin(), m_fields.end(),
                             [&](const CbFieldInfo& info) { return info.Name == fieldName; });
            it != m_fields.end())
        {
            auto& fieldInfo = *it;
            const auto original = BytesFromField(fieldInfo);
            if (AsCspan(original) == bytes)
                return;
            m_isDirty = true;

            Ensures(bytes.size() == fieldInfo.Size);
            gsl::copy(bytes, gsl::make_span(m_bytes).subspan(fieldInfo.Start, fieldInfo.Size));
        }
        else
        {
            throw std::invalid_argument{"Invalid field name!"};
        }
    }

    gsl::span<const std::byte> ConstantBufferInfo::GetBytes(std::string_view name) const
    {
        const auto pInfo = FindByName(name);
        // TODO:
        assert(pInfo != nullptr);
        return BytesFromField(*pInfo);
    }

    gsl::span<std::byte> ConstantBufferInfo::GetBytesMut(std::string_view name)
    {
        const auto pInfo = FindByName(name);
        // TODO:
        assert(pInfo != nullptr);
        m_isDirty = true;
        return BytesFromField(*pInfo);
    }

    std::string_view ConstantBufferInfo::Name() const { return m_name; }

    gsl::span<const std::byte> ConstantBufferInfo::Bytes() const { return gsl::make_span(m_bytes); }

    gsl::span<std::byte> ConstantBufferInfo::Bytes() { return gsl::make_span(m_bytes); }

    ConstantBufferInfo::ConstantBufferInfo(std::string name, std::uint32_t size,
                                           std::vector<CbFieldInfo> fieldInfos,
                                           std::uint32_t cbIndex)
        : m_name{std::move(name)}, m_fields{std::move(fieldInfos)}, m_cbIndex{cbIndex}, m_isDirty{true}
    {
        m_bytes.resize(size);
    }

    gsl::span<std::byte> ConstantBufferInfo::BytesFromField(const CbFieldInfo& pInfo)
    {
        return Bytes().subspan(pInfo.Start, pInfo.Size);
    }

    gsl::span<const std::byte> ConstantBufferInfo::BytesFromField(const CbFieldInfo& pInfo) const
    {
        return Bytes().subspan(pInfo.Start, pInfo.Size);
    }

    const CbFieldInfo* ConstantBufferInfo::FindByName(std::string_view name) const
    {
        if (const auto it =
                std::find_if(m_fields.begin(), m_fields.end(),
                             [&](const CbFieldInfo& info) { return info.Name == name; });
            it != m_fields.end())
        {
            return &*it;
        }
        return nullptr;
    }

    wrl::ComPtr<ID3D11ShaderReflection> ReflectShader(gsl::span<const std::byte> byteCode)
    {
        void* reflection;
        TryHR(
            D3DReflect(byteCode.data(), byteCode.size(), IID_ID3D11ShaderReflection, &reflection));
        wrl::ComPtr<ID3D11ShaderReflection> result;
        result.Attach(static_cast<ID3D11ShaderReflection*>(reflection));
        return result;
    }

#define BIND_WITH_PREFIX(prefix)                                                              \
    const auto& inputs = shader.Inputs;                                                       \
    const auto samplers = inputs.Samplers();                                                  \
    context3D.CONCAT(prefix, SetSamplers)(0, gsl::narrow<std::uint32_t>(samplers.size()),     \
                                          samplers.data());                                   \
    const auto views = inputs.ResourceViews();                                                \
    context3D.CONCAT(prefix, SetShaderResources)(0, gsl::narrow<std::uint32_t>(views.size()), \
                                                 views.data());                               \
    const auto cbs = inputs.Buffers();                                                        \
    context3D.CONCAT(prefix, SetConstantBuffers)(0, gsl::narrow<std::uint32_t>(cbs.size()),   \
                                                 cbs.data());                                  \
    context3D.CONCAT(prefix, SetShader)(shader.Shader.Get(), nullptr, 0);

    void Bind(ID3D11DeviceContext& context3D, const VertexShader& shader) { BIND_WITH_PREFIX(VS) }

    void Bind(ID3D11DeviceContext& context3D, const PixelShader& shader) { BIND_WITH_PREFIX(PS) }

    void SetupShaders(ID3D11DeviceContext& context3D, const ShaderCollection& shaders)
    {
        shaders.VertexShader_.Inputs.Flush(context3D);
        shaders.PixelShader_.Inputs.Flush(context3D);
        Bind(context3D, shaders.VertexShader_);
        Bind(context3D, shaders.PixelShader_);
        // TODO: HullShaders
    }

    ShaderCollection::ShaderCollection(VertexShader vs, PixelShader ps)
        : VertexShader_{std::move(vs)}, PixelShader_{std::move(ps)}
    {}

    } // namespace dx
