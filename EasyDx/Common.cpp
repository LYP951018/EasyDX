#include "pch.hpp"
#include <codecvt>
#include <comdef.h>
#include <stdexcept>
#include <Windows.h>
#include <d3d11.h>

namespace dx
{
    std::string ws2s(const std::wstring & wstr)
    {
        return std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>{}.to_bytes(wstr);
    }

    std::wstring s2ws(const std::string& str)
    {
        return std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>{}.from_bytes(str);
    }

    void TryHR(long hr)
    {
        if (FAILED(hr))
            ThrowHRException(hr);
    }

    void TryWin32(int b)
    {
        if (b != TRUE)
            throw std::system_error{ {static_cast<int>(::GetLastError()), std::system_category()} };
    }

    
    gsl::span<std::byte> BlobToSpan(ID3D10Blob& blob) noexcept
    {
        return { static_cast<std::byte*>(blob.GetBufferPointer()), static_cast<std::ptrdiff_t>(blob.GetBufferSize()) };
    }

    [[noreturn]]
    void ThrowHRException(HRESULT hr)
    {
        _com_error e{ hr };
        const auto errorMessage = e.ErrorMessage();
        throw std::runtime_error{ ws2s(errorMessage) };
    }

    Rect Rect::FromRECT(const ::RECT& win32Rect) noexcept
    {
        return { static_cast<std::uint32_t>(win32Rect.left),
            static_cast<std::uint32_t>(win32Rect.top),
            static_cast<std::uint32_t>(win32Rect.right - win32Rect.left),
            static_cast<std::uint32_t>(win32Rect.bottom - win32Rect.top) };
    }
}

