#pragma once

#include "Resources/Buffers.hpp"
#include "ComponentBase.hpp"
#include "Vertex.hpp"

namespace dx
{
    struct StreamInfo
    {
      public:
        StreamInfo(std::uint32_t stride) : Stride{stride}, IsDirty{true} {}

        gsl::span<std::byte> BytesSpan();
        gsl::span<const std::byte> BytesSpan() const;
        void UpdateBytesWithSameLength(gsl::span<const std::byte> bytes);
        void ResetBytes(gsl::span<const std::byte> bytes);

        const std::uint32_t Stride;
        mutable bool IsDirty;

      private:
        std::vector<std::byte> Bytes;
    };

    struct VbBindData
    {
        std::uint32_t* Strides;
        std::uint32_t* Offsets;
    };

    // TODO: Copy-on-write, BufferAllocator
    class Mesh : Noncopyable
    {
      public:
        enum DefaultStreamIds
        {
            kPositionStreamId = 0,
            kNormalStreamId = 1,
            kTexCoordStreamId = 2
        };

        DEFAULT_MOVE(Mesh)

        /*
           如果没有遵循 DefaultStreamIds 的约定请不要使用 Positions()、Normals() 以及 UVs()。
        */
        gsl::span<const PositionType> Positions() const
        {
            return GetStream<PositionType>(kPositionStreamId);
        }

        gsl::span<const VectorType> Normals() const
        {
            return GetStream<VectorType>(kNormalStreamId);
        }

        gsl::span<const TexCoordType> UVs() const
        {
            return GetStream<TexCoordType>(kTexCoordStreamId);
        }

        template<typename T>
        gsl::span<std::add_const_t<T>> GetStream(std::uint32_t streamId) const
        {
            const auto& stream = m_streams.at(streamId);
            Ensures(stream.Stride == sizeof(T));
            const auto bytes = stream.BytesSpan();
            const auto start = reinterpret_cast<std::add_const_t<T>*>(bytes.data());
            const auto end = reinterpret_cast<std::add_const_t<T>*>(bytes.data() + bytes.size());
            return {start, end};
        }

        void SetPositions(gsl::span<const PositionType> positions)
        {
            SetStream(kPositionStreamId, positions);
        }

        void SetNormals(gsl::span<const VectorType> normals)
        {
            SetStream(kNormalStreamId, normals);
        }

        void SetUVs(gsl::span<const TexCoordType> uvs) { SetStream(kTexCoordStreamId, uvs); }

        template<typename T>
        void SetStream(std::uint32_t streamId, gsl::span<T> vertices)
        {
            auto& stream = m_streams.at(streamId);
            Ensures(stream.Stride == sizeof(T));
            stream.UpdateBytesWithSameLength(gsl::as_bytes(vertices));
        }

        template<typename... Args, std::ptrdiff_t... N>
        void SetAllStreams(gsl::span<Args, N>... vertices)
        {
            const auto sizes = std::array{sizeof(Args)...};
            bool equals = std::equal(
                m_streams.begin(), m_streams.end(), sizes.begin(), sizes.end(),
                [](const StreamInfo& info, std::size_t size) { return info.Stride == size; });
            Ensures(equals);
            const auto streamsInBytes = std::array{gsl::as_bytes(vertices)...};
            SetAllStreamsInternal(gsl::make_span(streamsInBytes));
        }

        gsl::span<const GpuBuffer> GetGpuVbsWithoutFlush() const;
        ID3D11Buffer& GpuIb() const;
        std::uint32_t VertexCount() const;
        std::uint32_t IndexCount() const;
        ID3D11InputLayout& InputLayout() const;
        const VbBindData& BindData() const;

        //TODO: optimize flush
        void FlushAll(ID3D11DeviceContext& context3D) const;
        void FlushStream(ID3D11DeviceContext& context3D, std::uint32_t streamId) const;

        template<typename... Args, std::ptrdiff_t... N>
        static std::shared_ptr<Mesh>
        CreateImmutable(ID3D11Device& device, wrl::ComPtr<ID3D11InputLayout> inputLayout,
                        gsl::span<const ShortIndex> indices, gsl::span<Args, N>... vertices)
        {
            const auto channelsInBytes = std::array{gsl::span<const std::byte, -1>{gsl::as_bytes(vertices)}...};
            const auto strides = std::array{gsl::narrow<std::uint32_t>(sizeof(Args))...};
            return std::make_shared<Mesh>(CreateImmutableInternal(
                device, gsl::make_span(channelsInBytes), gsl::make_span(strides), indices, std::move(inputLayout)));
        }

        static Mesh CreateDynamic(ID3D11Device& device, gsl::span<VertexUnit> units,
                                  wrl::ComPtr<ID3D11InputLayout> layout);

      private:
        Mesh() = default;
        Mesh(std::vector<GpuBuffer> gpuBuffer, std::vector<StreamInfo> streams,
             wrl::ComPtr<ID3D11InputLayout> inputLayout, std::vector<std::uint32_t> stridesAndOffsets,
             GpuBuffer indexBuffer, std::uint32_t indexCount, bool isImmutable);

        static Mesh CreateImmutableInternal(ID3D11Device& device,
                                            gsl::span<const gsl::span<const std::byte>> bytes,
                                            gsl::span<const std::uint32_t> strides,
                                            gsl::span<const ShortIndex> indices,
                                            wrl::ComPtr<ID3D11InputLayout> inputLayout);

        void SetAllStreamsInternal(gsl::span<const gsl::span<const std::byte>> streamsInBytes);
        bool AnyDirty() const;

        std::vector<GpuBuffer> m_gpuVertexBuffers;
        GpuBuffer m_indexBuffer;
        wrl::ComPtr<ID3D11InputLayout> m_inputLayout;

        std::vector<StreamInfo> m_streams;
        VbBindData m_bindData;
        std::vector<std::uint32_t> m_stridesAndOffsets;
        std::uint32_t m_indexCount;
        bool m_isImmutable;

        friend void Bind(ID3D11DeviceContext& context3D, const Mesh& mesh);
    };

    // TODO: a submesh

    // setup input layout; set up gpu buffers
    void Bind(ID3D11DeviceContext& context3D, const Mesh& mesh);
    void SetupVertexBuffer(ID3D11DeviceContext& context3D, const Mesh& mesh);
} // namespace dx