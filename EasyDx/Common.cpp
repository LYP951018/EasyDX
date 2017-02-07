#include "Common.hpp"
#include <codecvt>
#include <comdef.h>
#include <stdexcept>

namespace dx
{
    std::string ws2s(const std::wstring & wstr)
    {
        return std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>{}.to_bytes(wstr);
    }

    void TryHR(long hr)
    {
        if (FAILED(hr))
            ThrowHRException(hr);
    }

    [[noreturn]]
    void ThrowHRException(HRESULT hr)
    {
        _com_error e{ hr };
        const auto errorMessage = e.ErrorMessage();
        throw std::runtime_error{ ws2s(errorMessage) };
    }
}

