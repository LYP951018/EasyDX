#include "pch.hpp"
#include "Mesh.hpp"
#include "Bind.hpp"
#include "Model.hpp"

namespace dx
{
    gsl::span<std::byte> StreamInfo::BytesSpan()
    {
        return gsl::make_span(Bytes);
    }

    gsl::span<const std::byte> StreamInfo::BytesSpan() const
    {
        return gsl::make_span(Bytes);
    }

    void StreamInfo::UpdateBytesWithSameLength(gsl::span<const std::byte> bytes)
    {
        const auto existingBytes = BytesSpan();
        Ensures(existingBytes.size() == bytes.size());
        gsl::copy(bytes, existingBytes);
    }

    void StreamInfo::ResetBytes(gsl::span<const std::byte> bytes)
    {
        Bytes = {bytes.begin(), bytes.end()};
    }

    gsl::span<const GpuBuffer> Mesh::GetGpuVbsWithoutFlush() const
    {
        return gsl::make_span(m_gpuVertexBuffers);
    }

    ID3D11Buffer& Mesh::GetGpuIndexBuffer() const { return Ref(m_indexBuffer); }

    std::uint32_t Mesh::GetVertexCount() const
    {
        const StreamInfo& streamInfo = m_streams[0];
        // FIXME
        return gsl::narrow<std::uint32_t>(streamInfo.BytesSpan().size() /
                                          streamInfo.GetStride());
    }

    std::uint32_t Mesh::GetIndexCount() const { return m_indexCount; }

    gsl::span<const D3D11_INPUT_ELEMENT_DESC>
    Mesh::GetFullInputElementDesces() const
    {
        return gsl::make_span(m_fullInputElementDesces);
    }

    D3D11_PRIMITIVE_TOPOLOGY Mesh::GetPrimitiveTopology() const
    {
        return m_primitiveTopology;
    }

    Mesh::Mesh(std::vector<GpuBuffer> gpuBuffer,
               std::vector<StreamInfo> streams,
               std::vector<D3D11_INPUT_ELEMENT_DESC> fullInputElementDesces,
               std::vector<VSSemantics> vsSemantics,
               std::vector<std::uint32_t> stridesAndOffsets,
               GpuBuffer indexBuffer, std::uint32_t indexCount,
               bool isImmutable, D3D_PRIMITIVE_TOPOLOGY topology,
               const DirectX::BoundingBox& boundingBox)
        : m_gpuVertexBuffers{std::move(gpuBuffer)}, m_indexBuffer{std::move(
                                                        indexBuffer)},
          m_fullInputElementDesces{std::move(fullInputElementDesces)},
          m_vsSemantics{std::move(vsSemantics)}, m_streams{std::move(streams)},
          m_stridesAndOffsets{std::move(stridesAndOffsets)},
          m_indexCount{indexCount}, m_isImmutable{isImmutable},
          m_primitiveTopology{topology}, m_boundingBox{boundingBox}
    {}

    Mesh Mesh::CreateImmutable(
        ID3D11Device& device, std::uint32_t channelCount,
        const gsl::span<const std::byte>* bytes, const std::uint32_t* strides,
        const VSSemantics* semantics,
        std::vector<D3D11_INPUT_ELEMENT_DESC> inputElementDesces,
        gsl::span<const ShortIndex> indices, D3D_PRIMITIVE_TOPOLOGY topology)
    {
        std::uint32_t offset = {};
        std::vector<StreamInfo> streams;
        std::vector<GpuBuffer> vertexBuffers;
        std::vector<std::uint32_t> stridesAndOffsets;
        streams.reserve(channelCount);
        vertexBuffers.reserve(channelCount);
        stridesAndOffsets.resize(channelCount * 2);
        std::uint32_t current = {};
        for (std::uint32_t i = 0; i < channelCount; ++i)
        {
            const gsl::span<const std::byte>& cpuVb = bytes[i];
            if (cpuVb.empty())
                continue;
            auto& stream = streams.emplace_back(StreamInfo{strides[i]});
            stream.ResetBytes(cpuVb);
            offset += gsl::narrow<std::uint32_t>(cpuVb.size());
            stridesAndOffsets[current] = strides[i];
            // TODO
            stridesAndOffsets[current + channelCount] = 0;
            ++current;
            vertexBuffers.push_back(MakeImmutableVertexBuffer(device, cpuVb));
        }
        DirectX::BoundingBox boundingBox;
        const std::uint32_t positionStride = strides[0];
        DirectX::BoundingBox::CreateFromPoints(
            boundingBox, bytes[0].size() / positionStride,
            reinterpret_cast<const DirectX::XMFLOAT3*>(bytes[0].data()),
            positionStride);
        GpuBuffer indexBuffer = MakeImmutableIndexBuffer(device, indices);
        std::vector<VSSemantics> vsSemantics{semantics,
                                             semantics + channelCount};
        return Mesh{std::move(vertexBuffers),
                    std::move(streams),
                    std::move(inputElementDesces),
                    std::move(vsSemantics),
                    std::move(stridesAndOffsets),
                    std::move(indexBuffer),
                    gsl::narrow<std::uint32_t>(indices.size()),
                    true,
                    topology,
                    boundingBox};
    }

    void Mesh::SetAllStreamsInternal(
        gsl::span<const gsl::span<const std::byte>> streamsInBytes)
    {
        for (std::ptrdiff_t i = 0; i < streamsInBytes.size(); ++i)
        {
            const auto& streamBytes = streamsInBytes[i];
            auto& stream = m_streams[i];
            stream.ResetBytes(streamBytes);
            stream.IsDirty = true;
        }
    }

    bool Mesh::AnyDirty() const
    {
        return std::any_of(
            m_streams.begin(), m_streams.end(),
            [](const StreamInfo& stream) { return stream.IsDirty; });
    }

    void Mesh::FlushAll(ID3D11DeviceContext& context3D) const
    {
        if (m_isImmutable)
            return;
        const auto streamCount = static_cast<std::uint32_t>(m_streams.size());
        for (std::uint32_t i = 0; i < streamCount; ++i)
        {
            auto& stream = m_streams[i];
            if (!stream.IsDirty)
            {
                continue;
            }
            FlushStream(context3D, i);
        }
    }

    void Mesh::FlushStream(ID3D11DeviceContext& context3D,
                           std::uint32_t streamId) const
    {
        const auto& stream = m_streams[streamId];
        UpdateWithDiscard(context3D, Ref(m_gpuVertexBuffers[streamId]),
                          stream.BytesSpan());
        stream.IsDirty = false;
    }

    void InputElementDescsFromMesh(
        std::vector<D3D11_INPUT_ELEMENT_DESC>& inputElementDesces,
        const Mesh& mesh, VSSemantics mask)
    {
        const gsl::span<const VSSemantics> semanticses = mesh.GetChannelMasks();
        const gsl::span<const D3D11_INPUT_ELEMENT_DESC> fullInputElementDesces =
            mesh.GetFullInputElementDesces();
        std::uint32_t inputLayoutIndex = 0;
        for (std::ptrdiff_t i = 0; i < semanticses.size(); ++i)
        {
            const VSSemantics semantics = semanticses[i];
            const std::uint32_t semanticsCount =
                __popcnt(static_cast<unsigned int>(semantics));
            if ((semantics & mask) != VSSemantics::kNone)
            {
                // g_VSSemantics.push_back(semantics);
                inputElementDesces.insert(
                    inputElementDesces.end(),
                    fullInputElementDesces.begin() + inputLayoutIndex,
                    fullInputElementDesces.begin() + inputLayoutIndex +
                        semanticsCount);
                mask &= ~semantics;
            }
            inputLayoutIndex += semanticsCount;
        }
    }

} // namespace dx
