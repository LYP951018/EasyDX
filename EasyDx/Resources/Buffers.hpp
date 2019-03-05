#pragma once

#include <d3d11.h>
#include <DirectXMath.h>

namespace dx
{
    struct MappedGpuResource : Noncopyable
    {
        MappedGpuResource(ID3D11DeviceContext& context, ID3D11Resource* resource, void* memory,
                          std::size_t size)
            : m_bytes{static_cast<std::byte*>(memory), static_cast<std::ptrdiff_t>(size)},
              m_context{&context}, m_resource{resource}
        {}

        // defend against usage like `buffer.Map().Bytes()`.
        gsl::span<std::byte> Bytes() const& { return m_bytes; }

        ~MappedGpuResource();

      private:
        ID3D11DeviceContext* m_context;
        ID3D11Resource* m_resource;
        gsl::span<std::byte> m_bytes;
    };

    struct GpuBufferView
    {
        ID3D11Buffer* Buffer;
        std::uint32_t Offset;
        std::uint32_t Count;
        std::uint32_t Stride;

        void Update(ID3D11DeviceContext& context, gsl::span<const std::byte> bytes);
    };

    namespace Internal
    {
        wrl::ComPtr<ID3D11Buffer> RawMakeD3DBuffer(::ID3D11Device& device, const void* buffer,
                                                   std::uint32_t bufferSize, BindFlag bindFlags,
                                                   ResourceUsage usage);
    }

    D3D11_BUFFER_DESC GetDesc(ID3D11Buffer& buffer);

    template<typename>
    using TypedGpuBuffer = wrl::ComPtr<ID3D11Buffer>;

    using GpuBuffer = TypedGpuBuffer<std::byte>;

    template<typename T>
    TypedGpuBuffer<T> MakeImmutableGpuBuffer(ID3D11Device& device, gsl::span<T> vertices,
                                             BindFlag bindFlags)
    {
        return Internal::RawMakeD3DBuffer(device, vertices.data(),
                                          gsl::narrow<std::uint32_t>(vertices.size_bytes()),
                                          bindFlags, ResourceUsage::Immutable);
    }

    template<typename T = std::byte>
    TypedGpuBuffer<T> MakeDynamicGpuBuffer(ID3D11Device& device, std::uint32_t size,
                                           BindFlag bindFlags, T* data = nullptr)
    {
        return Internal::RawMakeD3DBuffer(device, data, size, bindFlags, ResourceUsage::Dynamic);
    }

    template<typename T>
    TypedGpuBuffer<T> MakeImmutableVertexBuffer(ID3D11Device& device, gsl::span<T> vertices)
    {
        return MakeImmutableGpuBuffer(device, vertices, BindFlag::VertexBuffer);
    }

    template<typename T = std::byte>
    TypedGpuBuffer<T> MakeDynamicVertexBuffer(ID3D11Device& device, std::uint32_t size)
    {
        return MakeDynamicGpuBuffer(device, size, BindFlag::VertexBuffer);
    }

    template<typename T = std::byte>
    TypedGpuBuffer<T> MakeDynamicVertexBuffer(ID3D11Device& device, gsl::span<T> data)
    {
        return MakeDynamicGpuBuffer(device, data.size_bytes(), BindFlag::VertexBuffer, data.data());
    }

    using ShortIndex = std::uint16_t;
    using LongIndex = std::uint32_t;

    template<typename T>
    using is_index = is_one_of<std::decay_t<T>, ShortIndex, LongIndex>;

    template<typename T, typename = std::enable_if_t<is_index<T>::value>>
    TypedGpuBuffer<T> MakeImmutableIndexBuffer(ID3D11Device& device, gsl::span<T> vertices)
    {
        return MakeImmutableGpuBuffer(device, vertices, BindFlag::IndexBuffer);
    }

    template<typename T, typename = std::enable_if_t<is_index<T>::value>>
    TypedGpuBuffer<T> MakeDynamicIndexBuffer(ID3D11Device& device, gsl::span<T> vertices)
    {
        return MakeDynamicGpuBuffer(device, vertices, BindFlag::IndexBuffer);
    }

    MappedGpuResource Map(ID3D11DeviceContext& context, ID3D11Buffer& gpuBuffer,
                          ResourceMapType mapType = ResourceMapType::WriteDiscard);

    template<typename T>
    void UpdateWithDiscard(ID3D11DeviceContext& context, ID3D11Buffer& gpuBuffer,
                           gsl::span<T> vertices)
    {
        const auto desc = GetDesc(gpuBuffer);
        // Ensures(sizeof(T) == desc.StructureByteStride);
        Ensures(vertices.size_bytes() <= desc.ByteWidth);
        auto resource = Map(context, gpuBuffer, ResourceMapType::WriteDiscard);
        gsl::copy(gsl::as_bytes(vertices), resource.Bytes());
    }

    template<typename T>
    wrl::ComPtr<ID3D11Buffer> MakeConstantBuffer(ID3D11Device& device, T* cb = nullptr,
                                                 ResourceUsage usage = ResourceUsage::Dynamic)
    {
        return {Internal::RawMakeD3DBuffer(device, cb, gsl::narrow<std::uint32_t>(sizeof(*cb)),
                                           BindFlag::ConstantBuffer, usage)};
    }

    inline wrl::ComPtr<ID3D11Buffer> MakeConstantBuffer(ID3D11Device& device, std::uint32_t size)
    {
        return {Internal::RawMakeD3DBuffer(device, nullptr, size, BindFlag::ConstantBuffer,
                                           ResourceUsage::Dynamic)};
    }

    void SetupIndexBuffer(ID3D11DeviceContext& context3D, ID3D11Buffer& indexBuffer,
                          std::uint32_t offset = 0);
} // namespace dx