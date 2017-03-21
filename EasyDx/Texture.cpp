#include "Texture.hpp"
#include <DirectXTex.h>
#include <d3d11.h>

namespace dx
{
    wrl::ComPtr<ID3D11Texture2D> Texture::Load2DWicFromFile(ID3D11Device& device, const fs::path& filePath, ResourceUsage usage)
    {
        auto image = std::make_unique<DirectX::ScratchImage>();
        DirectX::TexMetadata metaData;
        TryHR(DirectX::LoadFromWICFile(filePath.c_str(),
            DirectX::WIC_FLAGS_NONE, &metaData, *image));
        wrl::ComPtr<ID3D11Resource> resource;
        TryHR(DirectX::CreateTextureEx(&device, image->GetImages(), image->GetImageCount(), metaData, static_cast<D3D11_USAGE>(usage), D3D11_BIND_SHADER_RESOURCE, 0, 0, 0, resource.ReleaseAndGetAddressOf()));
        wrl::ComPtr<ID3D11Texture2D> texture;
        const auto hr = resource.As(&texture);
        if (hr == E_NOINTERFACE)
            throw std::runtime_error{ "Invalid 2D WIC image" };
        TryHR(hr);
        return texture;
    }

    wrl::ComPtr<ID3D11Texture2D> Texture::Load2DFromMemory(ID3D11Device& device, const unsigned char* buffer, std::uint32_t width, std::uint32_t height, ResourceUsage usage)
    {
        D3D11_TEXTURE2D_DESC textureDesc = {};
        textureDesc.Width = width;
        textureDesc.Height = height;
        textureDesc.MipLevels = 1;
        textureDesc.ArraySize = 1;
        textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.SampleDesc.Quality = 0;
        textureDesc.Usage = static_cast<D3D11_USAGE>(usage);
        textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        textureDesc.CPUAccessFlags = 0;
        textureDesc.MiscFlags = 0;

        wrl::ComPtr<ID3D11Texture2D> d3dTexture;
        D3D11_SUBRESOURCE_DATA data = {};
        data.pSysMem = buffer;
        data.SysMemPitch = width * 4;
        data.SysMemSlicePitch = width * height * 4;
        TryHR(device.CreateTexture2D(&textureDesc, &data, d3dTexture.ReleaseAndGetAddressOf()));

        return d3dTexture;
    }

    wrl::ComPtr<ID3D11ShaderResourceView> Texture::GetView(ID3D11Device& device, ID3D11Texture2D& texture)
    {
        D3D11_SHADER_RESOURCE_VIEW_DESC desc = {};
        D3D11_TEXTURE2D_DESC textureDesc = {};
        texture.GetDesc(&textureDesc);
        desc.Format = textureDesc.Format;
        desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        desc.Texture2D = { 0, textureDesc.MipLevels };
        wrl::ComPtr<ID3D11ShaderResourceView> view;
        TryHR(device.CreateShaderResourceView(&texture, &desc, view.ReleaseAndGetAddressOf()));
        return view;
    }
}
