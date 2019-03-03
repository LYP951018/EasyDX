#pragma once

#include <cstdint>

namespace dx
{
    //https://msdn.microsoft.com/en-us/library/windows/desktop/aa383751(v=vs.85).aspx
    using NativeHandle = void*;

    inline constexpr auto MinusOneHandle = static_cast<std::intptr_t>(-1);
}