#pragma once

#include <d3d11.h>
#include <DirectXMath.h>

namespace dx
{
    namespace Internal
    {
        wrl::ComPtr<ID3D11Buffer> RawMakeD3DBuffer(
            ::ID3D11Device& device,
            const void* buffer,
            std::size_t bufferSize,
            BindFlag bindFlags,
            ResourceUsage usage);

        struct D3DBufferWrapper
        {
        public:
            ID3D11Buffer * D3DBuffer() const noexcept { return d3dBuffer_.Get(); }

        protected:
            D3DBufferWrapper(wrl::ComPtr<ID3D11Buffer> buffer)
                : d3dBuffer_{ std::move(buffer) }
            {}

            D3DBufferWrapper() = default;

        private:
            wrl::ComPtr<ID3D11Buffer> d3dBuffer_;
        };

    }

    void UpdateGpuBuffer(ID3D11DeviceContext& context, ID3D11Buffer& buffer, gsl::span<const std::byte> bytes);

    using Index = std::uint16_t;

    //FIXME: we should add DXGI_FORMAT to this class.
    struct ConstIndexBuffer : private Internal::D3DBufferWrapper
    {
        inline static constexpr DxgiFormat kFormat = DxgiFormat::R16UInt;

        ConstIndexBuffer(::ID3D11Device& device, gsl::span<const Index> indices, ResourceUsage usage = ResourceUsage::Default)
            : D3DBufferWrapper{ Internal::RawMakeD3DBuffer(device, indices.data(), static_cast<std::size_t>(indices.size_bytes()), BindFlag::IndexBuffer, usage) },
            countOfIndices_{ gsl::narrow<std::uint32_t>(indices.size()) }
        {}

        ConstIndexBuffer() = default;

        using Internal::D3DBufferWrapper::D3DBuffer;

        std::uint32_t CountOfIndices() const noexcept { return countOfIndices_; }

    private:
        std::uint32_t countOfIndices_;
    };


    struct VertexBuffer : private Internal::D3DBufferWrapper
    {
        VertexBuffer() = default;

        //construct from size only, dynamic.
        VertexBuffer(ID3D11Device& device, std::size_t vertexCount, std::size_t vertexSize, ResourceUsage usage = ResourceUsage::Default)
            : D3DBufferWrapper{ Internal::RawMakeD3DBuffer(device, nullptr, vertexCount * vertexSize, BindFlag::VertexBuffer, usage) },
            vertexCount_{ vertexCount },
            vertexSize_{ vertexSize }
        {}

        template<typename VertexT>
        VertexBuffer(ID3D11Device& device, gsl::span<VertexT> vertices, ResourceUsage usage = ResourceUsage::Immutable)
            : D3DBufferWrapper{ Internal::RawMakeD3DBuffer(device, vertices.data(), vertices.size_bytes(), BindFlag::VertexBuffer, usage) },
            vertexCount_{ static_cast<std::size_t>(vertices.size()) },
            vertexSize_{ sizeof(VertexT) }
        {}

        template<typename VertexT>
        void Update(ID3D11DeviceContext& context, gsl::span<VertexT> vertices)
        {
            Expects(static_cast<std::size_t>(vertices.size()) < vertexCount_);
            Expects(vertexSize == sizeof(VertexT));
            UpdateGpuBuffer(context, *D3DBuffer(), gsl::as_bytes(vertices));
        }

        std::uint32_t Stride() const noexcept
        {
            return gsl::narrow<std::uint32_t>(vertexSize_);
        }

        using Internal::D3DBufferWrapper::D3DBuffer;

    private:
        std::size_t vertexCount_;
        std::size_t vertexSize_;
    };

    template<typename>
    struct is_span : std::false_type {};

    template<typename T>
    struct is_span<gsl::span<T>> : std::true_type {};

    namespace Internal
    {
        template<std::size_t... I, typename... VertexT>
        void FillConstVbsImpl(ID3D11Device& device, gsl::span<VertexBuffer> vbs, std::index_sequence<I...>, gsl::span<VertexT>... vertices)
        {
            ((vbs[I] = VertexBuffer{ device, vertices }), ...);
        }
    }

    template<typename... VertexT>
    void FillConstVbs(ID3D11Device& device, gsl::span<VertexBuffer> vbs, gsl::span<VertexT>... vertices)
    {
        Internal::FillConstVbsImpl(device, vbs, std::index_sequence_for<VertexT...>{}, vertices...);
    }

    template<typename... ContTs, typename = std::enable_if_t<std::disjunction_v<std::negation<is_span<ContTs>>...>>>
    void FillConstVbs(ID3D11Device& device, gsl::span<VertexBuffer> vbs, const ContTs&... conts)
    {
        return FillConstVbs(device, vbs, gsl::make_span(conts)...);
    }

    template<typename T>
    wrl::ComPtr<ID3D11Buffer> MakeConstantBuffer(
        ID3D11Device& device,
        T* cb = nullptr,
        ResourceUsage usage = ResourceUsage::Default)
    {
        return { Internal::RawMakeD3DBuffer(device, cb, sizeof(*cb), BindFlag::ConstantBuffer, usage) };
    }


    template<typename T>
    struct ConstantBuffer : private Internal::D3DBufferWrapper
    {
        static_assert(std::is_standard_layout_v<T>);
    public:
        ConstantBuffer(ID3D11Device& device, ResourceUsage usage = ResourceUsage::Default)
            : D3DBufferWrapper{ MakeConstantBuffer<T>(device, nullptr, usage) }
        {}

        void Update(ID3D11DeviceContext& context, const T& data)
        {
            //FIXEME: Do we need to do this?
            static_assert(sizeof(T) % 16 == 0);
            UpdateGpuBuffer(context, *D3DBuffer(), AsBytes(data));
        }

        using Internal::D3DBufferWrapper::D3DBuffer;
    };

    template<std::size_t N>
    struct VbBinder
    {
        std::array<Ptr<ID3D11Buffer>, N> Buffers;
        std::uint32_t StartSlot;
        std::array<std::uint32_t, N> Strides;
        std::array<std::uint32_t, N> Offsets;
    };

    template<std::size_t N>
    VbBinder<N> MakeVbBinder(const std::array<VertexBuffer, N>& vbs)
    {
        VbBinder<N> binder{};
        for (std::size_t i = 0; i < vbs.size(); ++i)
        {
            auto& vb = vbs[i];
            binder.Buffers[i] = vb.D3DBuffer();
            binder.Strides[i] = vb.Stride();
            //TODO
            binder.Offsets[i] = {};
        }
        return binder;
    }

    template<std::size_t N>
    void Bind(ID3D11DeviceContext& context3D, const VbBinder<N>& binder)
    {
        context3D.IASetVertexBuffers(binder.StartSlot, N, binder.Buffers.data(), binder.Strides.data(), binder.Offsets.data());
    }

    void Bind(ID3D11DeviceContext& context3D, const ConstIndexBuffer& ib);
}