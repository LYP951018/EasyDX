#include "../pch.hpp"
#include "InputLayout.hpp"
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

    wrl::ComPtr<ID3D11InputLayout>
    InputLayoutAllocator::Register(ID3D11Device& device,
                                   gsl::span<const D3D11_INPUT_ELEMENT_DESC> descs,
                                   gsl::span<const std::byte> byteCode)
    {
        wrl::ComPtr<ID3D11InputLayout> layout;
        TryHR(device.CreateInputLayout(descs.data(), gsl::narrow<std::uint32_t>(descs.size()), byteCode.data(), byteCode.size(),
                                       layout.GetAddressOf()));
        m_inputLayouts.insert(std::make_pair(
            MaxStreamVector<D3D11_INPUT_ELEMENT_DESC>{descs.begin(), descs.end()}, layout));
        return layout;
    }

    wrl::ComPtr<ID3D11InputLayout>
    InputLayoutAllocator::Query(gsl::span<const D3D11_INPUT_ELEMENT_DESC> descs) const
    {
        if (const auto iter = m_inputLayouts.find(
                descs,
                [](const gsl::span<const D3D11_INPUT_ELEMENT_DESC>& v) {
                    return boost::hash_range(v.begin(), v.end());
                },
                [](const gsl::span<const D3D11_INPUT_ELEMENT_DESC>& rhs,
                   const MaxStreamVector<D3D11_INPUT_ELEMENT_DESC>& lhs) {
                    return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
                });
            iter != m_inputLayouts.end())
        {
            return iter->second;
        }
        //TODO
        assert(false);
    }
} // namespace dx