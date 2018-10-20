#pragma once

#include "Resources/Buffers.hpp"

namespace dx
{
    class Mesh;
    struct Material;
    struct Pass;

    void DrawMesh(ID3D11DeviceContext& context3D, const Mesh& mesh, const Material& material);
    void RunPasses(ID3D11DeviceContext& context3D, const dx::Material& material,
                   const dx::Mesh& mesh);
    void DrawMeshInstancing(ID3D11DeviceContext& context3D, const Mesh& mesh,
                            const Material& material, std::uint32_t instancingCount, gsl::span<const GpuBuffer> instancingBuffers, 
        gsl::span<const std::uint32_t> strides);
    void SetupBlending(ID3D11DeviceContext& context3D, const Pass& pass);
    void SetupDepthStencilStates(ID3D11DeviceContext& context3D, const Pass& pass);
    void SetupRasterizerState(ID3D11DeviceContext& context3D, const Pass& pass);
    void SetupPass(ID3D11DeviceContext& context3D, const Pass& pass);
} // namespace dx