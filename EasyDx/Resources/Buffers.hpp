#pragma once

#include <d3d11.h>

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
            ID3D11Buffer* D3DBuffer() const noexcept { return d3dBuffer_.Get(); }

        protected:
            D3DBufferWrapper(wrl::ComPtr<ID3D11Buffer> buffer)
                : d3dBuffer_{std::move(buffer)}
            {}

            D3DBufferWrapper() = default;

        private:
            wrl::ComPtr<ID3D11Buffer> d3dBuffer_;
        };

    }

    void UpdateGpuBuffer(ID3D11DeviceContext& context, ID3D11Buffer& buffer, gsl::span<const std::byte> bytes);

    template<typename VertexT>
    struct ImmutableVertexBuffer : private Internal::D3DBufferWrapper
    {
        inline static constexpr std::size_t kStride = sizeof(VertexT);
        using VbMarker = void;

        //construct from existing vertices
        template<typename T, std::ptrdiff_t N, typename = std::enable_if_t<std::is_same_v<std::remove_const_t<T>, VertexT>>>
        ImmutableVertexBuffer(ID3D11Device& device, gsl::span<T, N> vertices, ResourceUsage usage = ResourceUsage::Default)
            : D3DBufferWrapper{Internal::RawMakeD3DBuffer(device, vertices.data(), static_cast<std::size_t>(vertices.size_bytes()), BindFlag::VertexBuffer, usage)}
        {}

        ImmutableVertexBuffer() = default;

        using D3DBufferWrapper::D3DBuffer;
    };

    template<typename VertexT, std::ptrdiff_t N>
    ImmutableVertexBuffer(ID3D11Device& device, gsl::span<VertexT, N> vertices, ResourceUsage usage = ResourceUsage::Default)-> ImmutableVertexBuffer<std::remove_cv_t<VertexT>>;

    using Index = std::uint16_t;

    //FIXME: we should add DXGI_FORMAT to this class.
    struct ImmutableIndexBuffer : private Internal::D3DBufferWrapper
    {
        inline static constexpr DxgiFormat kFormat = DxgiFormat::R16UInt;

        ImmutableIndexBuffer(::ID3D11Device& device, gsl::span<const Index> indices, ResourceUsage usage = ResourceUsage::Default)
            : D3DBufferWrapper{ Internal::RawMakeD3DBuffer(device, indices.data(), static_cast<std::size_t>(indices.size_bytes()), BindFlag::IndexBuffer, usage) },
            countOfIndices_{gsl::narrow<std::uint32_t>(indices.size())}
        {}

        ImmutableIndexBuffer() = default;

        using D3DBufferWrapper::D3DBuffer;

        std::uint32_t CountOfIndices() const noexcept { return countOfIndices_; }

    private:
        std::uint32_t countOfIndices_;
    };


    //provide type-safe interface for vertex buffer.
    template<typename VertexT>
    struct DynamicVertexBuffer : private Internal::D3DBufferWrapper
    {
       inline static  constexpr std::size_t kStride = sizeof(VertexT);
       using VbMarker = void;

        //construct from size only, dynamic.
        DynamicVertexBuffer(ID3D11Device& device, std::size_t vertexCount)
            : D3DBufferWrapper{Internal::RawMakeD3DBuffer(device, nullptr, size * sizeof(VertexT), BindFlag::VertexBuffer, ResourceUsage::Dynamic)},
            vertexCount_{ vertexCount }
        {}

        void Update(ID3D11DeviceContext& context, gsl::span<VertexT> vertices)
        {
            Expects(static_cast<std::size_t>(vertices.size()) < vertexCount_);
            UpdateGpuBuffer(context, Ref(d3dBuffer_), gsl::as_bytes(vertices));
        }

        using D3DBufferWrapper::D3DBuffer;

    private:
        std::size_t vertexCount_;
    };

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
            : D3DBufferWrapper{MakeConstantBuffer<T>(device, nullptr, usage)}
        {}

        void Update(ID3D11DeviceContext& context, const T& data)
        {
            //FIXEME: Do we need to do this?
            static_assert(sizeof(T) % 16 == 0);
            UpdateGpuBuffer(context, Ref(cb.GpuCb), AsBytes(data));
        }

        using D3DBufferWrapper::D3DBuffer;
    };

    template<typename... BuffersT>
    std::tuple<BuffersT...> MakeVbSet(BuffersT... buffers)
    {
        return { std::move(buffers)... };
    }

    template<std::size_t N>
    struct VbBinder
    {
        std::uint32_t StartSlot;
        std::array<Ptr<ID3D11Buffer>, N> Buffers;
        std::array<std::uint32_t, N> Strides;
        std::array<std::uint32_t, N> Offsets;

        template<typename... BufferT, typename = std::enable_if_t<std::conjunction_v<is_vb<BufferT>...>>>
        VbBinder(std::tuple<BufferT...> vbSet, std::uint32_t startSlot = 0)
            : 
            StartSlot{ startSlot },
            Buffers{ std::apply(
                [](auto&... buffer) {
                    return std::array<Ptr<ID3D11Buffer>, sizeof... (buffer)>{ buffer.D3DBuffer()... }; }, vbSet) },
            Strides{BufferT::kStride...},
            //Offsets is TODO.
            Offsets{}
        {}

        //I could not figure out why clang could not decompose this struct.
        auto AsTuple() const
        {
            return std::tie(StartSlot, Buffers, Strides, Offsets);
        }
    };


    template<typename... T>
    VbBinder(std::tuple<T...> buffers, std::uint32_t startSlot = 0)-> VbBinder<sizeof...(T)>;

    template<std::size_t N>
    void Bind(ID3D11DeviceContext& context3D, const VbBinder<N>& binder)
    {
        const auto&[startSlot, buffers, strides, offsets] = binder.AsTuple();
        context3D.IASetVertexBuffers(startSlot, N, buffers.data(), strides.data(), offsets.data());
    }

    template<typename BufferT, typename = std::enable_if_t<is_vb_v<BufferT>>>
    void Bind(ID3D11DeviceContext& context3D, const BufferT& buffer)
    {
        Bind(context3D, VbBinder(MakeVbSet(buffer)));
    }

    void Bind(ID3D11DeviceContext& context3D, const ImmutableIndexBuffer& ib);
}