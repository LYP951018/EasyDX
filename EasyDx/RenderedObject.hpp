#pragma once

#include "Mesh.hpp"
#include "Common.hpp"
#include "Shaders.hpp"
#include <memory>
#include <vector>
#include <gsl/span>
#include <DirectXMath.h>

namespace dx
{
    class RenderedObject
    {
    public:
        //是否应该提供默认构造函数？
        RenderedObject() = default;
        RenderedObject(gsl::span<Mesh> meshes);

        RenderedObject(const RenderedObject&) = delete;
        RenderedObject& operator= (const RenderedObject&) = delete;

        RenderedObject(RenderedObject&&) = default;
        RenderedObject& operator= (RenderedObject&&) = default;

        DirectX::XMFLOAT3 Scale{ 1.f, 1.f, 1.f };
        DirectX::XMFLOAT4 Rotation{ 0.f, 0.f, 0.f, 1.f };
        DirectX::XMFLOAT3 Translation{ 0.f, 0.f, 0.f };

        DirectX::XMMATRIX ComputeWorld() const noexcept;
        void AttachVertexShader(VertexShader vs);
        void AttachPixelShader(wrl::ComPtr<ID3D11PixelShader> ps);
        void Render(ID3D11DeviceContext& deviceContext);

    private:
        std::vector<Mesh> meshes_;
        VertexShader vertexShader_;
        wrl::ComPtr<ID3D11PixelShader> pixelShader_;
    };
}