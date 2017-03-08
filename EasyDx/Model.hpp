#pragma once

#include "FileSystemAlias.hpp"
#include "Common.hpp"
#include "Mesh.hpp"
#include <vector>

namespace dx
{
    void LoadFromFile(ID3D11Device& device, const fs::path& filePath, std::vector<Mesh>& meshes);
}