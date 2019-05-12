#pragma once

#include "Resources/Buffers.hpp"

namespace dx
{
    class Mesh;
    struct Material;
    struct Pass;
    class ShaderInputs;
    enum class VSSemantics;

    void DrawMesh(ID3D11DeviceContext& context3D, const Mesh& mesh,
                  const Material& material, ID3D11Device* deviceToCreateInputLayout = nullptr);
    void DrawMesh(ID3D11DeviceContext& context3D, const Mesh& mesh,
                  const Pass& pass, ID3D11Device* deviceToCreateInputLayout = nullptr);
    void DrawMeshInstancing(ID3D11DeviceContext& context3D, const Mesh& mesh,
                            const Pass& pass, std::uint32_t instancingCount,
                            gsl::span<const GpuBuffer> instancingBuffers,
                            gsl::span<const std::uint32_t> strides);

} // namespace dx