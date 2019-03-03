#pragma once

#include <DirectXMath.h>

namespace dx
{
    class ShaderInputs;
    class CbFieldInfo;

    class GlobalShaderContext
    {
      public:
        DirectX::XMMATRIX ViewMatrix;
        DirectX::XMMATRIX ProjMatrix;
        DirectX::XMMATRIX ViewProjMatrix;

        void Flush();

      private:
        friend class Shader;
        void Apply(const std::unordered_map<std::string, gsl::span<std::byte>>& bytesMap) const;
    };
} // namespace dx