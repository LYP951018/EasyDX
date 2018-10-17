#include "pch.hpp"
#include "MeshRenderer.hpp"
#include "Mesh.hpp"
#include "Material.hpp"
#include "Bind.hpp"

namespace dx
{
    MeshRenderer::MeshRenderer(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material)
        : m_mesh{std::move(mesh)}, m_material{std::move(material)}
    {}

    Mesh& MeshRenderer::GetMesh() const { return *m_mesh; }
    Material& MeshRenderer::GetMaterial() const { return *m_material; }
} // namespace dx
