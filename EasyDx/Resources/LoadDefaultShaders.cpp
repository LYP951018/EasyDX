#include "../pch.hpp"
#include "Shaders.hpp"

namespace dx
{
    void Shaders::LoadDefaultShaders(ID3D11Device& device3D)
    {
        const auto currentPath = fs::current_path();

		const auto addWithName = [&](const char* name, const wchar_t* shaderFileName)
		{
			Add(name, Shader::FromCompiledCso(device3D, currentPath / shaderFileName));
		};
		addWithName(kBasicLighting, L"BasicPS.cso");
		addWithName(kPosNormalTexTransform, L"BasicVS.cso");
		addWithName(kQuadVS, L"UITextureVS.cso");
		addWithName(kQuadVS, L"UITexturePS.cso");
		addWithName(kDefaultShadowCasterVS, L"DefaultShadowCasterVS.cso");
		addWithName(kDefaultShadowCasterPS, L"DefaultShadowCasterPS.cso");
    }
} // namespace dx