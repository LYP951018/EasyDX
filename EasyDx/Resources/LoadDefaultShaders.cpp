#include "../pch.hpp"
#include "Shaders.hpp"

namespace dx
{
    void Shaders::LoadDefaultShaders(ID3D11Device& device3D)
    {
        const auto currentPath = fs::current_path();
        Add(kBasicLighting, Shader::FromCompiledCso(device3D, currentPath / L"BasicPS.cso"));
        Add(kPosNormalTexTransform,
            Shader::FromCompiledCso(device3D, currentPath / L"BasicVS.cso"));
        Add(kQuadVS, Shader::FromCompiledCso(device3D, currentPath / L"UITextureVS.cso"));
        Add(kQuadVS, Shader::FromCompiledCso(device3D, currentPath / L"UITexturePS.cso"));
    }
} // namespace dx