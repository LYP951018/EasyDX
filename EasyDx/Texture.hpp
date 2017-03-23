#pragma once

#include "Common.hpp"
#include "FileSystemAlias.hpp"
#include <memory>

namespace dx
{
    class Texture
    {
    public:
        Texture() = delete;

        static wrl::ComPtr<ID3D11Texture2D> Load2DFromWicFile(ID3D11Device& device, const fs::path& filePath, ResourceUsage usage = ResourceUsage::Default);
        static wrl::ComPtr<ID3D11Texture2D> Load2DFromTgaFile(ID3D11Device& device, const fs::path& filePath, ResourceUsage usage = ResourceUsage::Default);
        static wrl::ComPtr<ID3D11Texture2D> Load2DFromMemory(ID3D11Device& device, const unsigned char*, std::uint32_t width, std::uint32_t height, ResourceUsage usage = ResourceUsage::Default);
        static wrl::ComPtr<ID3D11Texture2D> Load2DFromFile(ID3D11Device& device, const fs::path& filePath, ResourceUsage usage = ResourceUsage::Default);
        static wrl::ComPtr<ID3D11ShaderResourceView> GetView(ID3D11Device& device, ID3D11Texture2D& texture);
    };
}