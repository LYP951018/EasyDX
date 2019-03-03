#pragma once

#include "ComponentBase.hpp"

namespace dx
{
    class Mesh;
    struct Material;

    class MeshRenderer : public ComponentBase
    {
      public:
        MeshRenderer(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material);

        Mesh& GetMesh() const;
        std::shared_ptr<Mesh> SharedMesh() const { return m_mesh; }
        Material& GetMaterial() const;

      private:
        std::shared_ptr<Mesh> m_mesh;
        std::shared_ptr<Material> m_material;
    };
} // namespace dx