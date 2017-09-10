#include "pch.hpp"
#include "Mesh.hpp"
#include <d3d11.h>

namespace dx
{
    template struct CpuMesh<SimpleVertex>;
    template struct CpuMeshView<SimpleVertex>;

    void SetupGpuMesh(ID3D11DeviceContext& context, GpuMeshView mesh)
    {
        SetupVertexBuffer(context, mesh.VertexBuffer);
        context.IASetIndexBuffer(mesh.IndexBuffer, DXGI_FORMAT_R16_UINT, 0);
    }
}
