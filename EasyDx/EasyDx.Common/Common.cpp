#include "Common.hpp"
#include <gsl/gsl_util>
#include <comdef.h>
#include <stdexcept>
#include <Windows.h>
#include <d3d11.h>

namespace dx
{
    std::string ws2s(std::wstring_view wstr)
    {
        if (wstr.empty())
            return {};
        const int inputSize = gsl::narrow<int>(wstr.size());
        const int sizeRequired =
            ::WideCharToMultiByte(CP_UTF8, {}, wstr.data(), inputSize, nullptr, 0, {}, {});
        Ensures(sizeRequired > 0);
        std::string result;
        result.resize(static_cast<std::size_t>(sizeRequired));
        if (::WideCharToMultiByte(CP_UTF8, {}, wstr.data(), inputSize, result.data(), sizeRequired,
                                  {}, {}) == 0)
        {
            ThrowWin32();
        }
        return result;
    }

    std::wstring s2ws(std::string_view str)
    {
        if (str.empty())
            return {};
        const int inputSize = gsl::narrow<int>(str.size());
        const int sizeRequired =
            ::MultiByteToWideChar(CP_UTF8, {}, str.data(), inputSize, nullptr, 0);
        Ensures(sizeRequired > 0);
        std::wstring result;
        result.resize(static_cast<std::size_t>(sizeRequired));
        if (::MultiByteToWideChar(CP_UTF8, {}, str.data(), inputSize, result.data(),
                                  sizeRequired) == 0)
        {
            ThrowWin32();
        }
        return result;
    }

    void ThrowWin32()
    {
        throw std::system_error{{static_cast<int>(::GetLastError()), std::system_category()}};
    }

    void TryHR(long hr)
    {
        if (FAILED(hr))
            ThrowHRException(hr);
    }

    void TryWin32(int b)
    {
        if (b != TRUE)
        {
            ThrowWin32();
        }
    }

    gsl::span<const std::byte> AsSpan(ID3D10Blob& blob)
    {
        return {static_cast<const std::byte*>(blob.GetBufferPointer()),
                gsl::narrow<std::ptrdiff_t>(blob.GetBufferSize())};
    }

    bool NearEqual(float lhs, float rhs)
    {
        return std::abs(lhs - rhs) < std::numeric_limits<float>::epsilon();
    }

    [[noreturn]] void ThrowHRException(HRESULT hr)
    {
        _com_error e{hr};
        const auto errorMessage = std::wstring_view{e.ErrorMessage()};
        throw std::runtime_error{ws2s(errorMessage)};
    }

    IntRect IntRect::FromRECT(const ::RECT& win32Rect) noexcept
    {
        return {static_cast<std::uint32_t>(win32Rect.left),
                static_cast<std::uint32_t>(win32Rect.top),
                static_cast<std::uint32_t>(win32Rect.right - win32Rect.left),
                static_cast<std::uint32_t>(win32Rect.bottom - win32Rect.top)};
    }
} // namespace dx
