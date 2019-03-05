#include "../pch.hpp"
#include "InputLayout.hpp"
#include "../Vertex.hpp"
#include "Shaders.hpp"
#include <d3d11.h>

bool operator==(const D3D11_INPUT_ELEMENT_DESC& lhs, const D3D11_INPUT_ELEMENT_DESC& rhs)
{
    return std::string_view{lhs.SemanticName} == std::string_view{rhs.SemanticName} &&
           lhs.SemanticIndex == rhs.SemanticIndex && lhs.Format == rhs.Format &&
           lhs.InputSlot == rhs.InputSlot && lhs.AlignedByteOffset == rhs.AlignedByteOffset &&
           lhs.InputSlotClass == rhs.InputSlotClass &&
           lhs.InstanceDataStepRate == rhs.InstanceDataStepRate;
}

bool operator!=(const D3D11_INPUT_ELEMENT_DESC& lhs, const D3D11_INPUT_ELEMENT_DESC& rhs)
{
    return !(lhs == rhs);
}

std::size_t hash_value(const D3D11_INPUT_ELEMENT_DESC& desc)
{
    return boost::hash_value(std::forward_as_tuple(
        std::string_view{desc.SemanticName}, desc.SemanticIndex, desc.Format, desc.InputSlot,
        desc.AlignedByteOffset, desc.InputSlotClass, desc.InstanceDataStepRate));
}

namespace dx
{
    void Bind(ID3D11DeviceContext& context3D, ::ID3D11InputLayout& layout)
    {
        context3D.IASetInputLayout(&layout);
    }

    std::unique_ptr<InputLayoutAllocator> g_inputAllocator;

    wrl::ComPtr<ID3D11InputLayout> MakeInputLayout(ID3D11Device& device,
                                                   gsl::span<const D3D11_INPUT_ELEMENT_DESC> descs,
                                                   gsl::span<const std::byte> byteCode)
    {
        wrl::ComPtr<ID3D11InputLayout> layout;
        TryHR(device.CreateInputLayout(descs.data(), gsl::narrow<std::uint32_t>(descs.size()),
                                       byteCode.data(), byteCode.size(), layout.GetAddressOf()));
        return layout;
    }

    wrl::ComPtr<ID3D11InputLayout>
    InputLayoutAllocator::Register(ID3D11Device& device,
                                   const gsl::span<const D3D11_INPUT_ELEMENT_DESC>& desc,
                                   const fs::path& csoPath)
    {
        const auto cso = MemoryMappedCso{csoPath};
        return Register(device, desc, cso.Bytes());
    }

    wrl::ComPtr<ID3D11InputLayout>
    InputLayoutAllocator::Register(ID3D11Device& device,
                                   gsl::span<const D3D11_INPUT_ELEMENT_DESC> descs,
                                   gsl::span<const std::byte> byteCode)
    {
        const auto layout = MakeInputLayout(device, descs, byteCode);
        g_inputAllocator->m_inputLayouts.insert(std::make_pair(
            MaxStreamVector<D3D11_INPUT_ELEMENT_DESC>{descs.begin(), descs.end()}, layout));
        return layout;
    }

    wrl::ComPtr<ID3D11InputLayout>
    InputLayoutAllocator::Query(gsl::span<const D3D11_INPUT_ELEMENT_DESC> descs)
    {
        if (const auto iter = g_inputAllocator->m_inputLayouts.find(
                descs,
                [](const gsl::span<const D3D11_INPUT_ELEMENT_DESC>& v) {
                    return boost::hash_range(v.begin(), v.end());
                },
                [](const gsl::span<const D3D11_INPUT_ELEMENT_DESC>& rhs,
                   const MaxStreamVector<D3D11_INPUT_ELEMENT_DESC>& lhs) {
                    return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
                });
            iter != g_inputAllocator->m_inputLayouts.end())
        {
            return iter->second;
        }
        return {};
    }

    void InputLayoutAllocator::Setup()
    {
        g_inputAllocator = std::make_unique<InputLayoutAllocator>();
    }

    void Arrange(gsl::span<D3D11_INPUT_ELEMENT_DESC> inputElementDesces)
    {
        for (std::ptrdiff_t i = 0; i < inputElementDesces.size(); ++i)
        {
            inputElementDesces[i].InputSlot = static_cast<std::uint32_t>(i);
        }
    }

    template<std::size_t N>
    void Gen(std::vector<D3D11_INPUT_ELEMENT_DESC>& inputElementDesces,
             std::array<VSSemantics, N> semanticses)
    {
        std::vector<DxgiFormat> formats;
        std::vector<std::uint32_t> semanticsIndices;
        for (const VSSemantics& semantics : semanticses)
        {
            formats.push_back(FormatFromSemantic(semantics));
            semanticsIndices.push_back(0);
        }
        FillInputElementsDesc(inputElementDesces, semanticses, formats, semanticsIndices);
    }

    // FIXME: too ad-hoc
    void InputLayoutAllocator::LoadDefaultInputLayouts(ID3D11Device& device3D)
    {
        const auto currentPath = fs::current_path();
        std::vector<D3D11_INPUT_ELEMENT_DESC> inputElementDesces;
        const auto registerHelper = [&](auto s, const fs::path& path) {
            Gen(inputElementDesces, s);
            Register(device3D, gsl::make_span(inputElementDesces), path);
        };
        registerHelper(std::array{VSSemantics::kPosition}, currentPath / L"PosVS.cso");
        registerHelper(
            std::array{VSSemantics::kPosition, VSSemantics::kNormal, VSSemantics::kTexCoord},
            currentPath / L"PosNormalTex.cso");
        registerHelper(std::array{VSSemantics::kPosition, VSSemantics::kNormal,
                                  VSSemantics::kTangent, VSSemantics::kTexCoord},
                       currentPath / L"PosNormTanTex.cso");
        // Register(device3D, QuadDescs, currentPath / L"UITextureVS.cso");
    }

    InputLayoutAllocator& InputLayoutAllocator::GetInstance() { return *g_inputAllocator; }
} // namespace dx