#pragma once

namespace dx
{
    wrl::ComPtr<ID3D11Texture2D> Load2DTexFromWicFile(ID3D11Device& device, const fs::path& filePath, ResourceUsage usage = ResourceUsage::Default);
    wrl::ComPtr<ID3D11Texture2D> Load2DTexFromTgaFile(ID3D11Device& device, const fs::path& filePath, ResourceUsage usage = ResourceUsage::Default);
    wrl::ComPtr<ID3D11Texture2D> Load2DTexFromDdsFile(ID3D11Device& device, const fs::path& filePath, ResourceUsage usage, std::uint32_t ddsFlags);
    wrl::ComPtr<ID3D11Texture2D> Load2DTexFromMemory(ID3D11Device& device, const unsigned char*, std::uint32_t width, std::uint32_t height, ResourceUsage usage = ResourceUsage::Default);
    wrl::ComPtr<ID3D11Texture2D> Load2DTexFromFile(ID3D11Device& device, const fs::path& filePath, ResourceUsage usage = ResourceUsage::Default);
    wrl::ComPtr<ID3D11ShaderResourceView> Get2DTexView(ID3D11Device& device, ID3D11Texture2D& texture);

    struct TextureSampler
    {
        wrl::ComPtr<ID3D11ShaderResourceView> Texture;
        wrl::ComPtr<ID3D11SamplerState> Sampler;
    };
}