#include "Common.hpp"
#include <codecvt>
#include <comdef.h>
#include <stdexcept>
#include <Windows.h>

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

    void* AlignedAlloc(std::size_t size, std::size_t align)
    {
#ifdef _MSC_VER 
        const auto ptr = _aligned_malloc(size, align);
        ThrowIf<std::bad_alloc>(ptr == nullptr);
        return static_cast<unsigned char*>(ptr);
#endif
    }

    void AlignedFree(void* ptr) noexcept
    {
        _aligned_free(ptr);
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

