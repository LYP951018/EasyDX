#pragma once

#include "Resources/Buffers.hpp"
#include "Vertex.hpp"

namespace dx
{
    template<typename T>
    struct CpuMesh 
    {
        static_assert(is_vertex_v<T>);
        std::vector<T> Vertices;
        std::vector<Index> Indices;
    };

    extern template struct CpuMesh<SimpleVertex>;

    using SimpleCpuMesh = CpuMesh<SimpleVertex>;
}