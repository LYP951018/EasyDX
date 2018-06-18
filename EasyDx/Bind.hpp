#pragma once

#include <type_traits>
#include "Resources.hpp"

namespace dx
{
    template<typename Bindable>
    using BindOp = decltype(dx::Bind(std::declval<ID3D11DeviceContext&>(), std::declval<Bindable&>()));

    template<typename T>
    using is_bindable = is_detected<BindOp, T>;

    template<typename T>
    inline constexpr bool is_bindable_v = is_bindable<T>::value;

    template<typename T1, typename T2, typename... BindableT, typename = std::enable_if_t<std::conjunction_v<is_bindable<T1&&>, is_bindable<T2&&>, is_bindable<BindableT&&>...>>>
    void Bind(ID3D11DeviceContext& context3D, T1&& arg1, T2&& arg2, BindableT&&... bindable)
    {
        dx::Bind(context3D, exforward(arg1));
        dx::Bind(context3D, exforward(arg2));
        (dx::Bind(context3D, exforward(bindable)), ...);
    }
}