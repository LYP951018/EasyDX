#include "pch.hpp"
#include "Vertex.hpp"

namespace dx
{
    D3D11_INPUT_ELEMENT_DESC MakeElementDesc(VSSemantics semantics,
                                             std::uint32_t inputSlot,
                                             DxgiFormat format,
                                             std::uint32_t semanticsIndex)
    {
        D3D11_INPUT_ELEMENT_DESC desc;
        desc.AlignedByteOffset = -1;
        desc.Format = static_cast<DXGI_FORMAT>(format);
        desc.InputSlot = inputSlot;
        desc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        desc.InstanceDataStepRate = 0;
        desc.SemanticIndex = semanticsIndex;
        desc.SemanticName = NameFromSemantic(semantics);
        return desc;
    }

    D3D11_INPUT_ELEMENT_DESC
    MakeInstancingElementDesc(VSSemantics semantics, std::uint32_t inputSlot,
                              DxgiFormat format, std::uint32_t stepRate,
                              std::uint32_t semanticsIndex)
    {
        D3D11_INPUT_ELEMENT_DESC desc;
        desc.AlignedByteOffset = -1;
        desc.Format = static_cast<DXGI_FORMAT>(format);
        desc.InputSlot = inputSlot;
        desc.InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
        desc.InstanceDataStepRate = stepRate;
        desc.SemanticIndex = semanticsIndex;
        desc.SemanticName = NameFromSemantic(semantics);
        return desc;
    }

    void FillInputElementsDesc(
        std::vector<D3D11_INPUT_ELEMENT_DESC>& inputElementsDesc,
        gsl::span<const VSSemantics> semanticses,
        gsl::span<const DxgiFormat> formats,
        gsl::span<const std::uint32_t> semanticsIndices)
    {
        inputElementsDesc.clear();
        std::uint32_t currentChannelIndex = 0;
        for (std::uint32_t i = 0; i < semanticses.size(); ++i)
        {
            std::uint32_t semantics =
                static_cast<std::uint32_t>(semanticses[i]);
            for (;;)
            {
                unsigned long index = 0;
                if (_BitScanForward(&index,
                                    static_cast<unsigned long>(semantics)) == 0)
                {
                    break;
                }
                inputElementsDesc.push_back(MakeElementDesc(
                    semanticses[i], i, formats[currentChannelIndex],
                    semanticsIndices[currentChannelIndex]));
                ++currentChannelIndex;
                semantics &= ~(1 << index);
            }
        }
    }

} // namespace dx
