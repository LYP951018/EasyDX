#pragma once

#include <d3d11.h>

bool operator==(const D3D11_INPUT_ELEMENT_DESC& lhs, const D3D11_INPUT_ELEMENT_DESC& rhs);
bool operator!=(const D3D11_INPUT_ELEMENT_DESC& lhs, const D3D11_INPUT_ELEMENT_DESC& rhs);

std::size_t hash_value(const D3D11_INPUT_ELEMENT_DESC& desc);

namespace boost
{
    template<typename T, std::size_t N>
    std::size_t hash_value(const boost::container::static_vector<T, N>& vec)
    {
        return hash_range(vec.begin(), vec.end());
    }

    template<typename T, std::size_t N>
    struct hash<boost::container::static_vector<T, N>>
    {
        size_t operator()(const boost::container::static_vector<T, N>& vec) const
        {
            return hash_value(vec);
        }
    };

}

namespace dx
{
    void Bind(ID3D11DeviceContext& context3D, ::ID3D11InputLayout& layout);

    wrl::ComPtr<ID3D11InputLayout> MakeInputLayout(ID3D11Device& device,
                                                   gsl::span<const D3D11_INPUT_ELEMENT_DESC> descs,
                                                   gsl::span<const std::byte> byteCode);

    class InputLayoutAllocator
    {
      public:
        template<std::size_t N>
        wrl::ComPtr<ID3D11InputLayout> Register(ID3D11Device& device,
                                                const std::array<D3D11_INPUT_ELEMENT_DESC, N>& desc,
                                                gsl::span<const std::byte> byteCode)
        {
            return Register(device, gsl::make_span(desc), byteCode);
        }

        wrl::ComPtr<ID3D11InputLayout> Register(ID3D11Device& device,
                                                 gsl::span<const D3D11_INPUT_ELEMENT_DESC> descs,
                                                 gsl::span<const std::byte> byteCode);
        wrl::ComPtr<ID3D11InputLayout> Query(gsl::span<const D3D11_INPUT_ELEMENT_DESC> descs) const;

      private:
        boost::unordered_map<MaxStreamVector<D3D11_INPUT_ELEMENT_DESC>,
                             wrl::ComPtr<ID3D11InputLayout>>
            m_inputLayouts;
    };
}

namespace boost
{

    template<>
    struct hash<D3D11_INPUT_ELEMENT_DESC>
    {
        size_t operator()(const D3D11_INPUT_ELEMENT_DESC& desc) const
        {
            return ::hash_value(desc);
        }
    };
}