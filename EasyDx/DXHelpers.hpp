#pragma once

#include "Common.hpp"
#include <cstdint>
#include <gsl/span>

namespace dx
{
    namespace Internal
    {
        extern wrl::ComPtr<ID3D11Buffer> RawMakeD3DBuffer(ID3D11Device& device, 
            const void* buffer, 
            std::size_t bufferSize,
            std::uint32_t bindFlags);
    }

    wrl::ComPtr<ID3D10Blob> CompileShaderFromFile(
        const wchar_t* fileName,
        const char* entryPoint,
        const char* shaderModel);

    wrl::ComPtr<ID3D11VertexShader> CreateVertexShader(ID3D11Device& device, ID3D10Blob& blob);
    wrl::ComPtr<ID3D11PixelShader> CreatePixelShader(ID3D11Device& device, ID3D10Blob& blob);

    template<typename VertexT>
    inline wrl::ComPtr<ID3D11Buffer> MakeVertexBuffer(ID3D11Device& device, gsl::span<VertexT> vertices)
    {
        return Internal::RawMakeD3DBuffer(device, vertices.data(), vertices.length_bytes(), 1);
    }

    template<typename IndexT>
    inline wrl::ComPtr<ID3D11Buffer> MakeIndexBuffer(ID3D11Device& device, gsl::span<IndexT> indices)
    {
        return Internal::RawMakeD3DBuffer(device, indices.data(), indices.length_bytes(), 2);
    }

    /*template<typename VertexT>
    inline wrl::ComPtr<ID3D11Buffer> MakeConstantBuffer(ID3D11Device& device, gsl::span<VertexT> vertices)
    {
        return Internal::RawMakeD3DBuffer(device, vertices.data(), vertices.length_bytes(), 4);
    }*/
}