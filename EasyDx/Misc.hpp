#pragma once

#include <cstdint>

namespace dx
{
    struct Point
    {
        std::int32_t X, Y;
    };

    struct Size
    {
        std::uint32_t Width, Height;
    };
}