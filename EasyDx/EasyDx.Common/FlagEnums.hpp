#pragma once

#include <type_traits>
#include "Detection.hpp"

namespace dx::Internal
{
    template<typename T>
    using UseEnumFlagOp = decltype(UseEnumFlag(std::declval<T>()));

    template<typename T>
    using EnableIfUseEnumFlag =
        std::enable_if_t<std::is_enum_v<T> && ::dx::is_detected_v<UseEnumFlagOp, T>>;
}

#define DEF_OPERATOR(op)                                                                     \
    template<typename T, typename = dx::Internal::EnableIfUseEnumFlag<T>>                    \
    inline constexpr T operator op(T lhs, T rhs)                                    \
    {                                                                                        \
        using Underlying = std::underlying_type_t<T>;                                        \
        return static_cast<T>(static_cast<Underlying>(lhs) op static_cast<Underlying>(rhs)); \
    }                                                                                        \
    template<typename T, typename = dx::Internal::EnableIfUseEnumFlag<T>>                    \
    inline constexpr T& operator op##=(T& lhs, T rhs)                               \
    {                                                                                        \
        lhs = lhs op rhs;                                                                    \
        return lhs;                                                                          \
    }

DEF_OPERATOR(|)
DEF_OPERATOR(&)

template<typename T, typename = dx::Internal::EnableIfUseEnumFlag<T>>
inline constexpr T operator~(T lhs)
{
    using Underlying = std::underlying_type_t<T>;
    return static_cast<T>(~static_cast<Underlying>(lhs));
}

#undef DEF_OPERATOR

#define ENABLE_FLAGS(type) \
    void UseEnumFlag(type);
