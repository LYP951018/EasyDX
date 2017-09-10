#pragma once

#include "DXDef.hpp"
#include "AlignedAllocator.hpp"
#include <string>
#include <wrl/client.h>

namespace wrl = Microsoft::WRL;

struct tagRECT;

namespace dx
{
    template<typename T>
    using Ptr = T*;

    template<typename T>
    using Rc = std::shared_ptr<T>;

    template<typename T>
    using Vec = std::vector<T>;

    template<typename T>
    using Box = std::unique_ptr<T>;

    std::string ws2s(const std::wstring& wstr);
    std::wstring s2ws(const std::string& str);

    [[noreturn]]
    void ThrowHRException(long hr);

    void TryHR(long hr);

    void TryWin32(int b);

    template<typename ExceptionT, typename... Args>
    void ThrowIf(bool cond, Args&&... args)
    {
        if (cond)
        {
            throw ExceptionT{ std::forward<Args>(args)... };
        }
    }

    template<typename T>
    T& Ref(const wrl::ComPtr<T>& ptr) noexcept
    {
        return *ptr.Get();
    }

    template<typename T>
    gsl::span<std::byte> AsBytes(T& object) noexcept
    {
        //static_assert(std::is_trivially_copyable_v<T>);
        return gsl::make_span(reinterpret_cast<std::byte*>(std::addressof(object)), sizeof(T));
    }

    template<typename T>
    gsl::span<const std::byte> AsBytes(const T& object) noexcept
    {
        //static_assert(std::is_trivially_copyable_v<T>);
        return gsl::make_span(reinterpret_cast<const std::byte*>(std::addressof(object)), sizeof(T));
    }

    template<std::size_t N>
    gsl::span<std::byte> AsBytes(unsigned char(& arr)[N])
    {
        const auto parr = reinterpret_cast<std::byte*>(arr);
        return gsl::make_span(parr, N);
    }

    template<std::size_t N>
    gsl::span<const std::byte> AsBytes(const unsigned char(&arr)[N])
    {
        const auto parr = reinterpret_cast<const std::byte*>(arr);
        return gsl::make_span(parr, N);
    }

    template<typename T>
    gsl::span<const Ptr<T>> ComPtrsCast(gsl::span<const wrl::ComPtr<T>> comPtrs) noexcept
    {
        static_assert(std::is_standard_layout_v<wrl::ComPtr<T>>, "wrl::ComPtr<T> should be a standard layout class.");
        return gsl::make_span(reinterpret_cast<Ptr<const Ptr<T>>>(reinterpret_cast<const char*>(comPtrs.data())), comPtrs.size());
    }

    template<typename T>
    gsl::span<T*> ComPtrsCast(gsl::span<wrl::ComPtr<T>> comPtrs) noexcept
    {
        static_assert(std::is_standard_layout_v<wrl::ComPtr<T>>, "wrl::ComPtr<T> should be a standard layout class.");
        return gsl::make_span(reinterpret_cast<Ptr<T*>>(reinterpret_cast<char*>(comPtrs.data())), comPtrs.size());
    }

    struct Rect
    {
        std::uint32_t LeftTopX, LeftTopY, Width, Height;
        static Rect FromRECT(const tagRECT& win32Rect) noexcept;
    };

    template<typename T>
    using aligned_unique_ptr = std::unique_ptr<T, void(&)(void*)>;

    template<typename T>
    aligned_unique_ptr<T> aligned_unique()
    {
        return { static_cast<std::add_pointer_t<T>>(stlext::AlignedAlloc(sizeof(T), alignof(T))), stlext::AlignedFree };
    }

    template<typename T>
    aligned_unique_ptr<T[]> aligned_unique(std::size_t n)
    {
        return { static_cast<std::add_pointer_t<T>>(stlext::AlignedAlloc(sizeof(T) * n, alignof(T))), stlext::AlignedFree };
    }

    gsl::span<std::byte> BlobToSpan(ID3D10Blob& blob) noexcept;

    struct Noncopyable
    {
        Noncopyable() = default;
        Noncopyable(const Noncopyable&) = delete;
        Noncopyable& operator= (const Noncopyable&) = delete;
        Noncopyable(Noncopyable&&) = default;
        Noncopyable& operator= (Noncopyable&&) = default;
    };

    template<typename T, typename... Args>
    std::shared_ptr<T> MakeShared(Args&&... args)
    {
        static_assert(std::is_constructible_v<T, Args&&...>);
        return std::make_shared<T>(std::forward<Args>(args)...);
    }

    template<typename T, typename... Args>
    auto MakeUnique(Args&&... args)->std::enable_if_t<std::is_constructible_v<T, Args&&...>,
        std::unique_ptr<T>>
    {
        return std::unique_ptr<T>(std::forward<Args>(args)...);
    }

    template<typename... Args>
    void Swallow(Args&&...) noexcept {}
}

namespace stlext
{
    template<typename T>
    using AlignedVec = std::vector<T, AlignAllocator<T>>;
}