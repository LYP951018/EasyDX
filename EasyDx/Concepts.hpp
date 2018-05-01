#pragma once

namespace dx
{
    template<typename Bindable>
    using BindOp = decltype(Bind(std::declval<ID3D11DeviceContext&>(), std::declval<Bindable&>()));

    template<typename T>
    using is_bindable = is_detected<BindOp, T>;

    template<typename T>
    inline constexpr bool is_bindable_v = is_bindable<T>::value;

    namespace Internal
    {
        template<typename T>
        using VbOp = typename T::VbMarker;
    }

    template<typename T>
    using is_vb = is_detected<Internal::VbOp, T>;

    DEF_INLINE_VAR_SINGLE(is_vb);
}