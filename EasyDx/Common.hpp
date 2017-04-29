#pragma once

#include "DXDef.hpp"
#include "AlignAllocator.h"
#include <string>
#include <gsl/span>
#include <type_traits>
#include <wrl/client.h>
#include <vector>

namespace wrl = Microsoft::WRL;

struct tagRECT;

namespace dx
{
    template<typename T>
    using Ptr = T*;

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
    gsl::byte* ToBytes(T* ptr) noexcept
    {
        return reinterpret_cast<gsl::byte*>(ptr);
    }

    template<typename T>
    const gsl::byte* ToBytes(const T* ptr) noexcept
    {
        return reinterpret_cast<const gsl::byte*>(ptr);
    }

    template<std::size_t N>
    gsl::span<gsl::byte> AsBytes(unsigned char(& arr)[N])
    {
        const auto parr = reinterpret_cast<gsl::byte*>(arr);
        return gsl::make_span(parr, N);
    }

    template<std::size_t N>
    gsl::span<const gsl::byte> AsBytes(const unsigned char(&arr)[N])
    {
        const auto parr = reinterpret_cast<const gsl::byte*>(arr);
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

    gsl::span<gsl::byte> BlobToSpan(ID3D10Blob& blob) noexcept;

    struct Noncopyable
    {
        Noncopyable() = default;
        Noncopyable(const Noncopyable&) = delete;
        Noncopyable& operator= (const Noncopyable&) = delete;
        Noncopyable(Noncopyable&&) = default;
        Noncopyable& operator= (Noncopyable&&) = default;
    };
}

namespace stlext
{
    template<typename T>
    using AlignedVec = std::vector<T, AlignAllocator<T>>;
}