#include "pch.hpp"
#include "Mesh.hpp"
#include "Bind.hpp"

namespace dx
{
    gsl::span<std::byte> StreamInfo::BytesSpan() { return gsl::make_span(Bytes); }

    gsl::span<const std::byte> StreamInfo::BytesSpan() const { return gsl::make_span(Bytes); }

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

    ID3D11Buffer& Mesh::GpuIb() const { return Ref(m_indexBuffer); }

    std::uint32_t Mesh::VertexCount() const
    {
        return gsl::narrow<std::uint32_t>(Positions().size());
    }

    std::uint32_t Mesh::IndexCount() const { return m_indexCount; }

    ID3D11InputLayout& Mesh::InputLayout() const { return Ref(m_inputLayout); }

    const VbBindData& Mesh::BindData() const { return m_bindData; }

    Mesh Mesh::CreateDynamic(ID3D11Device&, gsl::span<VertexUnit>,
                             wrl::ComPtr<ID3D11InputLayout>)
    {
        assert(false);
        // TODO
        Mesh mesh;
        /*mesh.m_isImmutable = true;
        mesh.m_streams.reserve(units.size());
        std::transform(
            units.begin(), units.end(), std::back_inserter(mesh.m_streams),
            [](const VertexUnit& unit) {
                return StreamInfo{0, gsl::narrow<std::uint32_t>(SizeFromFormat(unit.Format)), 0};
            });
        mesh.m_inputLayout = std::move(layout);*/
        return mesh;
    }

    Mesh::Mesh(std::vector<GpuBuffer> gpuBuffer, std::vector<StreamInfo> streams,
               wrl::ComPtr<ID3D11InputLayout> inputLayout,
               std::vector<std::uint32_t> stridesAndOffsets,
               GpuBuffer indexBuffer, std::uint32_t indexCount, bool isImmutable)
        : m_gpuVertexBuffers{std::move(gpuBuffer)}, m_indexBuffer{std::move(indexBuffer)},
          m_inputLayout{std::move(inputLayout)}, m_streams{std::move(streams)},
          m_stridesAndOffsets{std::move(stridesAndOffsets)}, m_indexCount{indexCount},
          m_isImmutable{isImmutable}
    {
        m_bindData.Strides = m_stridesAndOffsets.data();
        m_bindData.Offsets = m_stridesAndOffsets.data() + m_stridesAndOffsets.size() / 2;
    }

    Mesh Mesh::CreateImmutableInternal(ID3D11Device& device,
                                       gsl::span<const gsl::span<const std::byte>> cpuVertices,
                                       gsl::span<const std::uint32_t> strides,
                                       gsl::span<const ShortIndex> indices,
                                       wrl::ComPtr<ID3D11InputLayout> inputLayout)
    {
        std::uint32_t offset = {};
        std::vector<StreamInfo> streams;
        std::vector<GpuBuffer> vertexBuffers;
        std::vector<std::uint32_t> stridesAndOffsets;
        const auto channelCount = cpuVertices.size();
        streams.reserve(channelCount);
        vertexBuffers.resize(channelCount);
        stridesAndOffsets.resize(channelCount * 2);
        auto bindData = std::make_unique<VbBindData>();
        for (std::ptrdiff_t i = 0; i < cpuVertices.size(); ++i)
        {
            const auto& cpuVb = cpuVertices[i];
            auto& stream = streams.emplace_back(StreamInfo{strides[i]});
            stream.ResetBytes(cpuVb);
            offset += gsl::narrow<std::uint32_t>(cpuVb.size());
            stridesAndOffsets[i] = strides[i];
            //TODO
            stridesAndOffsets[i + channelCount] = 0;
            vertexBuffers[i] = MakeImmutableVertexBuffer(device, cpuVb);
        }
        auto indexBuffer = MakeImmutableIndexBuffer(device, indices);
        return Mesh{std::move(vertexBuffers),
                    std::move(streams),
                    std::move(inputLayout),
                    std::move(stridesAndOffsets),
                    std::move(indexBuffer),
                    gsl::narrow<std::uint32_t>(indices.size()),
                    true};
    }

    void Mesh::SetAllStreamsInternal(gsl::span<const gsl::span<const std::byte>> streamsInBytes)
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
        return std::any_of(m_streams.begin(), m_streams.end(),
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

    void Mesh::FlushStream(ID3D11DeviceContext& context3D, std::uint32_t streamId) const
    {
        const auto& stream = m_streams[streamId];
        UpdateWithDiscard(context3D, Ref(m_gpuVertexBuffers[streamId]), stream.BytesSpan());
        stream.IsDirty = false;
    }

} // namespace dx
