#pragma once

#include <type_traits>
#include "Resources/Buffers.hpp"
#include "Resources.hpp"

namespace dx
{
    template<typename T1, typename T2, typename... BindableT, typename = std::enable_if_t<std::conjunction_v<is_bindable<T1>, is_bindable<T2>, is_bindable<BindableT>...>>>
    void Bind(ID3D11DeviceContext& context3D, const T1& arg1, const T2& arg2, const BindableT&... bindable)
    {
        dx::Bind(context3D, arg1);
        dx::Bind(context3D, arg2);
        (dx::Bind(context3D, bindable), ...);
    }
}