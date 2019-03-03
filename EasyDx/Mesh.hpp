#pragma once

#include "Resources/Buffers.hpp"
#include "ComponentBase.hpp"
#include "Vertex.hpp"
#include <d3d11.h> //for D3D11_PRIMITIVE_TOPOLOGY

namespace dx
{
    constexpr std::uint32_t kMaxInputSlotCount = 8;

    struct StreamInfo
    {
      public:
        StreamInfo(std::uint32_t stride) : m_stride{stride}, IsDirty{true} {}

        std::uint32_t GetStride() const { return m_stride; }
        gsl::span<std::byte> BytesSpan();
        gsl::span<const std::byte> BytesSpan() const;
        void UpdateBytesWithSameLength(gsl::span<const std::byte> bytes);
        void ResetBytes(gsl::span<const std::byte> bytes);

        mutable bool IsDirty;

      private:
        std::uint32_t m_stride;
        std::vector<std::byte> Bytes;
    };

    // TODO: Copy-on-write, BufferAllocator
    class Mesh : Noncopyable
    {
      public:
        DEFAULT_MOVE(Mesh)

        template<typename T>
        gsl::span<std::add_const_t<T>> GetStream(std::uint32_t streamId) const
        {
            const StreamInfo& stream = m_streams.at(streamId);
            Ensures(stream.GetStride() == sizeof(T));
            const auto bytes = stream.BytesSpan();
            const auto start = reinterpret_cast<std::add_const_t<T>*>(bytes.data());
            const auto end = reinterpret_cast<std::add_const_t<T>*>(bytes.data() + bytes.size());
            return {start, end};
        }

        template<typename T>
        void SetStream(std::uint32_t streamId, gsl::span<T> vertices)
        {
            StreamInfo& stream = m_streams.at(streamId);
            Ensures(stream.GetStride() == sizeof(T));
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
        ID3D11Buffer& GetGpuIndexBuffer() const;
        std::uint32_t GetVertexCount() const;
        std::uint32_t GetIndexCount() const;
		gsl::span<const D3D11_INPUT_ELEMENT_DESC> GetFullInputElementDesces() const;
        D3D11_PRIMITIVE_TOPOLOGY GetPrimitiveTopology() const;
		gsl::span<const StreamInfo> GetStreamsInfo() const { return gsl::make_span(m_streams); }
		gsl::span<const VSSemantics> GetChannelMasks() const { return gsl::make_span(m_vsSemantics); }

        // TODO: optimize flush
        void FlushAll(ID3D11DeviceContext& context3D) const;
        void FlushStream(ID3D11DeviceContext& context3D, std::uint32_t streamId) const;

		//unowned
        template<typename... Args>
        static std::shared_ptr<Mesh>
        CreateImmutable(ID3D11Device& device, D3D11_PRIMITIVE_TOPOLOGY topology,
                        gsl::span<const ShortIndex> indices, const VSSemantics* semantics,
                        std::uint32_t vertexCount, std::vector<D3D11_INPUT_ELEMENT_DESC> inputElementDesces, Args*... vertices)
        {
            const auto channelsInBytes =
                std::array{gsl::as_bytes(gsl::make_span(vertices, vertexCount))...};
            const auto strides = std::array{gsl::narrow<std::uint32_t>(sizeof(Args))...};
            return std::make_shared<Mesh>(
                CreateImmutable(device, channelsInBytes.size(), channelsInBytes.data(),
                                        strides.data(), semantics, std::move(inputElementDesces), indices, topology));
        }

		//template<typename... Args>
		//static std::shared_ptr<Mesh>
		//	CreateImmutable(ID3D11Device& device, D3D11_PRIMITIVE_TOPOLOGY topology,
		//		gsl::span<const ShortIndex> indices, const VSSemantics* semantics,
		//		std::uint32_t vertexCount, std::vector<Args> ... vertices)
		//{
		//	CreateImmutable()
		//}

		static Mesh CreateImmutable(ID3D11Device& device, std::uint32_t channelCount,
			const gsl::span<const std::byte>* bytes,
			const std::uint32_t* strides,
			const VSSemantics* semantics,
			std::vector<D3D11_INPUT_ELEMENT_DESC> inputElementDesces,
			gsl::span<const ShortIndex> indices,
			D3D_PRIMITIVE_TOPOLOGY topology);

      private:
        Mesh() = default;
        Mesh(std::vector<GpuBuffer> gpuBuffer, std::vector<StreamInfo> streams,
				std::vector<D3D11_INPUT_ELEMENT_DESC> fullInputElementDesces,
             std::vector<VSSemantics> vsSemantics, std::vector<std::uint32_t> stridesAndOffsets,
             GpuBuffer indexBuffer, std::uint32_t indexCount, bool isImmutable,
             D3D_PRIMITIVE_TOPOLOGY topology);

        
        void SetAllStreamsInternal(gsl::span<const gsl::span<const std::byte>> streamsInBytes);
        bool AnyDirty() const;

        std::vector<GpuBuffer> m_gpuVertexBuffers;
        GpuBuffer m_indexBuffer;

        // TODO: 这里用 vector 太浪费了，倾向于 unique_ptr<T[]> + m_channelCount
        std::vector<StreamInfo> m_streams;
		std::vector<D3D11_INPUT_ELEMENT_DESC> m_fullInputElementDesces;
        std::vector<VSSemantics> m_vsSemantics;
        std::vector<std::uint32_t> m_stridesAndOffsets;
        std::uint32_t m_indexCount;
        bool m_isImmutable;
        D3D11_PRIMITIVE_TOPOLOGY m_primitiveTopology;
    };

    // TODO: a submesh

	//void InputElementDescsFromMesh(std::vector<D3D11_INPUT_ELEMENT_DESC>& inputElementDesces, const Mesh& mesh, VSSemantics semanticesToUse);
} // namespace dx