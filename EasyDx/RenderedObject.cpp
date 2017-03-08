#include "RenderedObject.hpp"
#include <utility>
#include <d3d11.h>

namespace dx
{
    RenderedObject::RenderedObject(gsl::span<Mesh> meshes)
        : meshes_{meshes.begin(), meshes.end()}
    {
    }

    void RenderedObject::AttachVertexShader(VertexShader vs)
    {
        vertexShader_ = std::move(vs);
    }

    void RenderedObject::AttachPixelShader(wrl::ComPtr<ID3D11PixelShader> ps)
    {
        pixelShader_ = std::move(ps);
    }

    DirectX::XMMATRIX RenderedObject::ComputeWorld() const noexcept
    {
        using namespace DirectX;
        return XMMatrixMultiply(XMMatrixMultiply(
            XMMatrixScaling(Scale.x, Scale.y, Scale.z),
            XMMatrixRotationQuaternion(XMLoadFloat4(&Rotation))),
            XMMatrixTranslation(Translation.x, Translation.y, Translation.z));
    }

    void RenderedObject::Render(ID3D11DeviceContext& deviceContext)
    {
        vertexShader_.Bind(deviceContext);
        deviceContext.PSSetShader(pixelShader_.Get(), nullptr, 0);
        for (auto& mesh : meshes_)
        {
            mesh.Render(deviceContext);
        }
    }
}