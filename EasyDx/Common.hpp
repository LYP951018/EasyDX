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

    [[noreturn]]
    void ThrowHRException(long hr);

    void TryHR(long hr);

    void TryWin32(int b);

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
}
