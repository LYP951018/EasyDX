#pragma once

#include <DirectXMath.h>
#include "CBStructs.hpp"

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
        DirectX::XMFLOAT3 EyePos;
        cb::Light lights[10];
        int lightCount;

        void Flush();

      private:
        friend class Shader;
        void Apply(const std::unordered_map<std::string, gsl::span<std::byte>>&
                       bytesMap) const;
    };
} // namespace dx