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
                            const Material& material, gsl::span<const GpuBuffer> instancingBuffers);
    void SetupBlending(ID3D11DeviceContext& context3D, const Pass& pass);
    void SetupDepthStencilStates(ID3D11DeviceContext& context3D, const Pass& pass);
    void SetupRasterizerState(ID3D11DeviceContext& context3D, const Pass& pass);
} // namespace dx