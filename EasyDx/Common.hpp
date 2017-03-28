#pragma once

#include "DXDef.hpp"
#include <string>
#include <gsl/span>
#include <type_traits>
#include <wrl/client.h>

namespace wrl = Microsoft::WRL;

struct tagRECT;

namespace dx
{
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
    gsl::span<T*> ComPtrsCast(gsl::span<wrl::ComPtr<T>> comPtrs) noexcept
    {
        static_assert(std::is_standard_layout_v<wrl::ComPtr<T>>, "wrl::ComPtr<T> should be a standard layout class.");
        //TODO: const?
        return gsl::make_span(reinterpret_cast<T*>(reinterpret_cast<char*>(comPtrs.data())), comPtrs.size());
    }

    struct Rect
    {
        std::uint32_t LeftTopX, LeftTopY, Width, Height;
        static Rect FromRECT(const tagRECT& win32Rect) noexcept;
    };

    void* AlignedAlloc(std::size_t size, std::size_t align);
    void AlignedFree(void* ptr) noexcept;

    template<typename T>
    using aligned_unique_ptr = std::unique_ptr<T, void(&)(void*)>;

    template<typename T>
    aligned_unique_ptr<T> aligned_unique()
    {
        return { static_cast<std::add_pointer_t<T>>(AlignedAlloc(sizeof(T), alignof(T))), AlignedFree };
    }
}
