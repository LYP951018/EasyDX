#pragma once

#include "DXDef.hpp"
#include <string>
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

    struct Rect
    {
        std::uint32_t LeftTopX, LeftTopY, Width, Height;
        static Rect FromRECT(const tagRECT& win32Rect) noexcept;
    };

}
