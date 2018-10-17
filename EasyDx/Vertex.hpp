#pragma once

#include "DXDef.hpp"
#include <DirectXMath.h>
#include "DxMathWrappers.hpp"
#include <d3d11.h>

namespace dx
{
    enum class VSSemantics
    {
        kBinormal,
        kColor,
        kNormal,
        kPosition,
        kTransformedPosition,
        kTangent,
        kTexCoord
    };

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
            break;
        }
    }
    constexpr std::size_t SizeFromFormat(DxgiFormat format)
    {
        switch (format)
        {
        case DxgiFormat::R32G32B32A32Float:
            return 16;
        case DxgiFormat::R32G32B32Float:
            return 12;
        case DxgiFormat::R16UInt:
        case DxgiFormat::R32G32Float:
            return 2;
        case DxgiFormat::B8G8R8A8UNorm:
            return 4;
        case DxgiFormat::Depth24UNormStencilS8UInt:
            return 4;
        default:
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

    // TODO: 这里可以做到不手动输入 input
    // slot，但是可能会引入模板元编程之类的劳什子……还是先手动指定吧。 理想状态： MakeDescArray(
    // Cell {
    //      MakeVertex(VSSemantics::kPosition),
    //      MakeVertex(VSSemantics::kNormal)
    // },
    //  Cell { MakeVertex(VSSemantics::kPosition) }
    //});
    struct VertexUnit
    {
        // construct a vertex unit from predefined semantics
        constexpr VertexUnit(VSSemantics semantic, std::uint32_t slot, InputSlotKind kind = {})
            : VertexUnit{NameFromSemantic(semantic), kind, FormatFromSemantic(semantic), slot}
        {}
        // construct a vertex unit from custom semantics
        constexpr VertexUnit(std::string_view name, InputSlotKind kind, DxgiFormat format,
                             std::uint32_t slot, std::uint32_t index = {},
                             std::uint32_t stepRate = {})
            : SemanticName{name},
              SemanticIndex{index}, SlotKind{kind}, Format{format}, StepRate{stepRate}, Slot{slot}
        {}

        std::string_view SemanticName;
        std::uint32_t SemanticIndex;
        InputSlotKind SlotKind;
        DxgiFormat Format;
        std::uint32_t StepRate;
        std::uint32_t Slot;
    };

    constexpr VertexUnit MakeInstancing(VSSemantics semantic, std::uint32_t slot)
    {
        return VertexUnit{semantic, slot, InputSlotKind::kPerinstance};
    }

    constexpr VertexUnit MakeInstancing(std::string_view name, std::uint32_t index,
                                        std::uint32_t slot,
                                        DxgiFormat format = DxgiFormat::R32G32B32A32Float,
                                        std::uint32_t stepRate = 1)
    {
        return VertexUnit{name, InputSlotKind::kPerinstance, format, slot, index, stepRate};
    }

    constexpr VertexUnit MakeVertex(VSSemantics semantic, std::uint32_t slot)
    {
        return VertexUnit{semantic, slot, InputSlotKind::kPervertex};
    }

    constexpr VertexUnit MakeVertex(std::string_view name, std::uint32_t slot,
                                    DxgiFormat format = DxgiFormat::R32G32B32A32Float)
    {
        return VertexUnit{name, InputSlotKind::kPervertex, format, slot};
    }

#define MATRIX_VERTEX_UNITS(name, slot)                                       \
    ::dx::MakeInstancing(name, 0, slot), ::dx::MakeInstancing(name, 1, slot), \
        ::dx::MakeInstancing(name, 2, slot), ::dx::MakeInstancing(name, 3, slot)

    // Due to Variadic macro bugs in MSVC, we could not write `VERTEX_UNITS` macro.

    // template<std::size_t N>
    // constexpr auto MakeUnits(std::array<VSSemantics, N> semantics) -> std::array<VertexUnit, N>
    //{
    //    return std::apply([](VSSemantics semantic) { return VertexUnit{ semantic }; }, semantics);
    //}

    template<std::ptrdiff_t N, typename = std::enable_if_t<N != gsl::dynamic_extent>>
    constexpr auto MakeDescArray(gsl::span<const VertexUnit, N> units)
        -> std::array<D3D11_INPUT_ELEMENT_DESC, N>
    {
        std::array<D3D11_INPUT_ELEMENT_DESC, N> result{};
        std::size_t offset = {};
        std::uint32_t previousSlot = 0;
        for (std::ptrdiff_t i = 0; i < units.size(); ++i)
        {
            const std::size_t si = static_cast<std::size_t>(i);
            const VertexUnit& unit = units[i];
            if (unit.Slot != previousSlot)
            {
                previousSlot = unit.Slot;
                offset = {};
            }
            const auto format = static_cast<DXGI_FORMAT>(unit.Format);
            const auto classification = static_cast<D3D11_INPUT_CLASSIFICATION>(unit.SlotKind);
            result[si] = D3D11_INPUT_ELEMENT_DESC{
                unit.SemanticName.data(), unit.SemanticIndex, format, unit.Slot,
                // gsl::narrow is not constexpr.
                D3D11_APPEND_ALIGNED_ELEMENT, classification, unit.StepRate};
            offset += SizeFromFormat(unit.Format);
        }
        return result;
    }

    template<std::size_t N>
    constexpr auto MakeDescArray(std::array<const VertexUnit, N> units)
        -> std::array<D3D11_INPUT_ELEMENT_DESC, N>
    {
        return MakeDescArray(gsl::span<const VertexUnit, N>{units});
    }

    template<std::size_t N>
    constexpr auto MakeDescArray(std::array<VertexUnit, N> units)
        -> std::array<D3D11_INPUT_ELEMENT_DESC, N>
    {
        return MakeDescArray(gsl::span<const VertexUnit, N>{units});
    }

    inline constexpr auto PosNormTexTanUnits =
        std::array{MakeVertex(VSSemantics::kPosition, 0), MakeVertex(VSSemantics::kNormal, 1),
                   MakeVertex(VSSemantics::kTexCoord, 2), MakeVertex(VSSemantics::kTangent, 3)};


    inline constexpr auto PosNormTexUnits =
        std::array{MakeVertex(VSSemantics::kPosition, 0), MakeVertex(VSSemantics::kNormal, 1),
                   MakeVertex(VSSemantics::kTexCoord, 2)};

    inline constexpr auto PosNormalTexDescs = MakeDescArray(PosNormTexUnits);

    inline constexpr auto PosDesc =
        MakeDescArray(std::array{MakeVertex(VSSemantics::kPosition, 0)});

    using PositionType = DirectX::XMFLOAT3A;
    using VectorType = DirectX::XMFLOAT3A;
    using TexCoordType = DirectX::XMFLOAT2A;
    using ColorType = DirectX::XMFLOAT4;

    using PositionStream = std::vector<PositionType>;
    using VectorStream = std::vector<VectorType>;
    using TexCoordStream = std::vector<TexCoordType>;
    using ColorStream = std::vector<ColorType>;

    PositionType MakePosition(float x, float y, float z);
    VectorType MakeDir(float x, float y, float z, float w = 0.0f);
    TexCoordType MakeTexCoord(float x, float y);
    ColorType MakeColor(float r, float g, float b, float a);

    inline VectorType StoreVec(DirectX::XMVECTOR vec) { return Store<VectorType>(vec); }

} // namespace dx