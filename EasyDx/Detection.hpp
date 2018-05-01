#pragma once

#include <type_traits>

//copyed from http://en.cppreference.com/w/cpp/experimental/is_detected
namespace dx
{
    namespace Internal 
    {
        template <class Default, class AlwaysVoid,
            template<class...> class Op, class... Args>
        struct detector 
        {
            using type = Default;
            static constexpr bool value = false;
        };

        template <class Default, template<class...> class Op, class... Args>
        struct detector<Default, std::void_t<Op<Args...>>, Op, Args...> : std::true_type
        {
            using type = Op<Args...>;
            static constexpr bool value = true;

        };
    }

    struct nonesuch
    {
        nonesuch() = delete;
        ~nonesuch() = delete;
        nonesuch(nonesuch const&) = delete;
        void operator=(nonesuch const&) = delete;
    };

    template <template<class...> class Op, class... Args>
    using is_detected = typename Internal::detector<nonesuch, void, Op, Args...>;

    template <template<class...> class Op, class... Args>
    using detected_t = typename Internal::detector<nonesuch, void, Op, Args...>::type;

    template <class Default, template<class...> class Op, class... Args>
    using detected_or = Internal::detector<Default, void, Op, Args...>;

    template< template<class...> class Op, class... Args >
    inline constexpr bool is_detected_v = is_detected<Op, Args...>::value;

    template< class Default, template<class...> class Op, class... Args >
    using detected_or_t = typename detected_or<Default, Op, Args...>::type;

    template <class Expected, template<class...> class Op, class... Args>
    using is_detected_exact = std::is_same<Expected, detected_t<Op, Args...>>;

    template <class Expected, template<class...> class Op, class... Args>
    inline constexpr bool is_detected_exact_v = is_detected_exact<Expected, Op, Args...>::value;

    template <class To, template<class...> class Op, class... Args>
    using is_detected_convertible = std::is_convertible<detected_t<Op, Args...>, To>;

    template <class To, template<class...> class Op, class... Args>
    inline constexpr bool is_detected_convertible_v = is_detected_convertible<To, Op, Args...>::value;
}