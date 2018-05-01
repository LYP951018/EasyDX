#include "../pch.hpp"
#include "InputLayout.hpp"
#include <d3d11.h>

namespace dx
{
    void Bind(ID3D11DeviceContext& context3D, ::ID3D11InputLayout& layout)
    {
        context3D.IASetInputLayout(&layout);
    }
}