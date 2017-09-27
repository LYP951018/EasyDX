#pragma once

#include "FileSystemAlias.hpp"
#include "Component.hpp"

namespace dx
{
    wrl::ComPtr<ID3D11Texture2D> Load2DTexFromWicFile(ID3D11Device& device, const fs::path& filePath, ResourceUsage usage = ResourceUsage::Default);
    wrl::ComPtr<ID3D11Texture2D> Load2DTexFromTgaFile(ID3D11Device& device, const fs::path& filePath, ResourceUsage usage = ResourceUsage::Default);
    wrl::ComPtr<ID3D11Texture2D> Load2DTexFromDdsFile(ID3D11Device& device, const fs::path& filePath, ResourceUsage usage, std::uint32_t ddsFlags);
    wrl::ComPtr<ID3D11Texture2D> Load2DTexFromMemory(ID3D11Device& device, const unsigned char*, std::uint32_t width, std::uint32_t height, ResourceUsage usage = ResourceUsage::Default);
    wrl::ComPtr<ID3D11Texture2D> Load2DTexFromFile(ID3D11Device& device, const fs::path& filePath, ResourceUsage usage = ResourceUsage::Default);
    wrl::ComPtr<ID3D11ShaderResourceView> Get2DTexView(ID3D11Device& device, ID3D11Texture2D& texture);

    struct Texture : IComponent
    {
    public:
        Texture(ID3D11Device& device,
            wrl::ComPtr<ID3D11Texture2D> tex,
            wrl::ComPtr<ID3D11SamplerState> sampler);

        ID3D11Texture2D& GetTex() const noexcept { return Ref(tex_); }
        ID3D11ShaderResourceView& GetView() const noexcept { return Ref(view_); }
        ID3D11SamplerState& GetSampler() const noexcept { return Ref(sampler_); }

        std::uint32_t GetId() const override
        {
            return ComponentId::kTexture;
        }

        static std::uint32_t GetStaticId()
        {
            return ComponentId::kTexture;
        }

    private:
        wrl::ComPtr<ID3D11Texture2D> tex_;
        wrl::ComPtr<ID3D11ShaderResourceView> view_;
        wrl::ComPtr<ID3D11SamplerState> sampler_;
    };
}