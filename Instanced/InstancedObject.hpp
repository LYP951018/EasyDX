#pragma once

#include <EasyDx/Common.hpp>
#include <EasyDx/Vertex.hpp>
#include <vector>
#include <gsl/span>
#include <d3d11.h>

template<typename VertexT,
    typename InstancedT>
    struct InstancingObject
{
    static_assert(dx::is_vertex_v<VertexT>);

    InstancingObject(std::uint32_t count)
    {

    }

    VertexT Vertex;

    void Draw(ID3D11DeviceContext& context)
    {

    }

    ID3D11Buffer* InstancedBuffer() const { return gpuInstancedData_.Get(); }

    gsl::span<InstancedT> InstancedData() { return gsl::make_span(cpuInstancedData_); }
    gsl::span<const InstancedT> InstancedData() const { return gsl::make_span(cpuInstancedData_); }

private:
    wrl::ComPtr<ID3D11Buffer> gpuInstancedData_;
    std::vector<InstancedT> cpuInstancedData_;

};