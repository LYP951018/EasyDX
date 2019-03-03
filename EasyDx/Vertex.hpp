#pragma once

#include "DXDef.hpp"
#include <DirectXMath.h>
#include "DxMathWrappers.hpp"
#include "EasyDx.Common/FlagEnums.hpp"
#include <d3d11.h>

namespace dx
{
    enum class VSSemantics
    {
		kNone = 0,
        kBinormal = 1,
        kColor = 1 << 1,
        kNormal = 1 << 2,
        kPosition = 1 << 3,
        kTransformedPosition = 1 << 4,
        kTangent = 1 << 5,
        kTexCoord = 1 << 6
    };

	ENABLE_FLAGS(VSSemantics)

    enum InputSlotKind
    {
        kPervertex = 0,
        kPerinstance = 1
    };

    constexpr const char* NameFromSemantic(VSSemantics semantic)
    {
        switch (semantic)
        {
        case VSSemantics::kBinormal:
            return u8"BINORMAL";
        case VSSemantics::kColor:
            return u8"COLOR";
        case VSSemantics::kNormal:
            return u8"NORMAL";
        case VSSemantics::kPosition:
            return u8"POSITION";
        case VSSemantics::kTransformedPosition:
            return u8"POSITIONT";
        case VSSemantics::kTangent:
            return u8"TANGENT";
        case VSSemantics::kTexCoord:
            return u8"TEXCOORD";
        default:
            throw std::invalid_argument{
                fmt::format("Invalid semantics {}.", static_cast<std::uint32_t>(semantic))};
            break;
        }
    }

	constexpr DxgiFormat FormatFromSemantic(VSSemantics semantic)
	{
		switch (semantic)
		{
		case VSSemantics::kBinormal:
		case VSSemantics::kColor:
		case VSSemantics::kNormal:
		case VSSemantics::kPosition:
		case VSSemantics::kTransformedPosition:
		case VSSemantics::kTangent:
			return DxgiFormat::R32G32B32A32Float;
		case VSSemantics::kTexCoord:
			return DxgiFormat::R32G32Float;
		default:
			break;
		}
	}

	D3D11_INPUT_ELEMENT_DESC MakeElementDesc(VSSemantics semantics, std::uint32_t inputSlot, DxgiFormat format, std::uint32_t semanticsIndex = 0);
	D3D11_INPUT_ELEMENT_DESC MakeInstancingElementDesc(VSSemantics semantics, std::uint32_t inputSlot, DxgiFormat format, std::uint32_t stepRate = 1, std::uint32_t semanticsIndex = 0);

	//TODO: instancing?
	void FillInputElementsDesc(std::vector<D3D11_INPUT_ELEMENT_DESC>& inputElementsDesc, 
		gsl::span<VSSemantics> semanticses,
		gsl::span<DxgiFormat> formats,
		gsl::span<std::uint32_t> semanticsIndices);

#define MATRIX_VERTEX_UNITS(name, slot)                                       \
    ::dx::MakeInstancing(name, 0, slot), ::dx::MakeInstancing(name, 1, slot), \
        ::dx::MakeInstancing(name, 2, slot), ::dx::MakeInstancing(name, 3, slot)

} // namespace dx