#pragma once

#include <filesystem>
#include <wrl.h>
#include <memory>
#include <string_view>
#include <cstddef>
#include <gsl/span>

namespace fs = std::filesystem;
namespace wrl = Microsoft::WRL;

struct tagRECT;
struct ID3D10Blob;

namespace dx
{
    template<typename T>
    using Ptr = T*;

    template<typename T>
    using Rc = std::shared_ptr<T>;

    template<typename T>
    using Vec = std::vector<T>;

    template<typename T>
    using Box = std::unique_ptr<T>;

    std::string ws2s(std::wstring_view wstr);
    std::wstring s2ws(std::string_view str);

    [[noreturn]] void ThrowHRException(long hr);

    [[noreturn]] void ThrowWin32();

    void TryHR(long hr);

    void TryWin32(int b);

    template<typename ExceptionT, typename... Args>
    void ThrowIf(bool cond, Args&&... args)
    {
        if (cond)
        {
            throw ExceptionT{std::forward<Args>(args)...};
        }
    }

    template<typename T>
    T& Ref(const wrl::ComPtr<T>& ptr) noexcept
    {
        return *ptr.Get();
    }

    template<typename T>
    gsl::span<std::byte> AsBytes(T& object) noexcept
    {
        static_assert(std::is_standard_layout_v<T>);
        return gsl::make_span(
            reinterpret_cast<std::byte*>(std::addressof(object)), sizeof(T));
    }

    template<typename T>
    gsl::span<const std::byte> AsBytes(const T& object) noexcept
    {
        static_assert(std::is_standard_layout_v<T>);
        return gsl::make_span(
            reinterpret_cast<const std::byte*>(std::addressof(object)),
            sizeof(T));
    }

    template<std::size_t N>
    gsl::span<std::byte> AsBytes(unsigned char (&arr)[N])
    {
        const auto parr = reinterpret_cast<std::byte*>(arr);
        return gsl::make_span(parr, N);
    }

    template<std::size_t N>
    gsl::span<const std::byte> AsBytes(const unsigned char (&arr)[N])
    {
        const auto parr = reinterpret_cast<const std::byte*>(arr);
        return gsl::make_span(parr, N);
    }

    template<typename T, std::ptrdiff_t N>
    gsl::span<const Ptr<T>, N>
    ComPtrsCast(gsl::span<const wrl::ComPtr<T>, N> comPtrs) noexcept
    {
        static_assert(std::is_standard_layout_v<wrl::ComPtr<T>>,
                      "wrl::ComPtr<T> should be a standard layout class.");
        static_assert(sizeof(wrl::ComPtr<T>) == sizeof(Ptr<T>),
                      "wrl::ComPtr<T> should have the same size as the "
                      "raw pointer.");
        const auto start = reinterpret_cast<const Ptr<T>*>(comPtrs.data());
        const auto last = start + comPtrs.size();
        return {start, last};
    }

    template<typename T, std::ptrdiff_t N>
    gsl::span<Ptr<T>, N>
    ComPtrsCast(gsl::span<wrl::ComPtr<T>, N> comPtrs) noexcept
    {
        static_assert(std::is_standard_layout_v<wrl::ComPtr<T>>,
                      "wrl::ComPtr<T> should be a standard layout class.");
        static_assert(sizeof(wrl::ComPtr<T>) == sizeof(Ptr<T>),
                      "wrl::ComPtr<T> should have the same size as the "
                      "raw pointer.");
        const auto start = reinterpret_cast<Ptr<T>*>(comPtrs.data());
        const auto last = start + comPtrs.size();
        return {start, last};
    }

    struct IntRect
    {
        std::uint32_t LeftTopX, LeftTopY, Width, Height;
        static IntRect FromRECT(const tagRECT& win32Rect) noexcept;
    };

    struct Noncopyable
    {
        Noncopyable() = default;
        Noncopyable(const Noncopyable&) = delete;
        Noncopyable& operator=(const Noncopyable&) = delete;
        Noncopyable(Noncopyable&&) = default;
        Noncopyable& operator=(Noncopyable&&) = default;
    };

    template<typename T, typename... Args>
    std::shared_ptr<T> MakeShared(Args&&... args)
    {
        static_assert(std::is_constructible_v<T, Args&&...>);
        return std::make_shared<T>(std::forward<Args>(args)...);
    }

    template<typename T, typename... Args>
    auto MakeUnique(Args&&... args)
        -> std::enable_if_t<std::is_constructible_v<T, Args&&...>,
                            std::unique_ptr<T>>
    {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }

    template<typename... Args>
    void Swallow(Args&&...) noexcept
    {}

    template<typename T>
    struct is_vertex : std::false_type
    {};

    template<typename T>
    constexpr bool is_vertex_v = is_vertex<T>::value;

    template<typename>
    struct always_false : std::false_type
    {};

    template<typename T, typename... Args>
    using is_one_of = std::disjunction<std::is_same<T, Args>...>;

    template<typename T, typename... Args>
    inline constexpr bool is_one_of_v = is_one_of<T, Args...>::value;

    template<typename T, typename... Args>
    T& construct_at(T* ptr, Args&&... args)
    {
        ::new (static_cast<void*>(ptr)) T(std::forward<Args>(args)...);
        return *ptr;
    }

    gsl::span<const std::byte> AsSpan(::ID3D10Blob& blob);

    template<typename T, std::ptrdiff_t N>
    gsl::span<const T> AsCspan(gsl::span<T, N> sp)
    {
        return sp;
    }

    template<typename T, typename... Rs>
    void Append(T& vec, const Rs&... r)
    {
        (vec.insert(vec.end(), r.begin(), r.end()), ...);
    }

    bool NearEqual(float lhs, float rhs);

    template<typename T, typename... Args>
    auto MakeVec(T&& t, Args&&... args)
        -> std::vector<std::remove_reference_t<T>>
    {
        std::vector<std::remove_reference_t<T>> vec;
        vec.reserve(sizeof...(Args) + 1);
        vec.push_back(std::forward<T>(t));
        (vec.push_back(std::forward<Args>(args)), ...);
        return vec;
    }

    template<typename T>
    gsl::span<T> SingleAsSpan(T& v)
    {
        return gsl::make_span(std::addressof(v), 1);
    }

    using Clock = std::chrono::high_resolution_clock;
    using Duration = Clock::duration;
    using TimePoint = Clock::time_point;
} // namespace dx

#define exforward(v) std::forward<decltype(v)>(v)

#define CONCAT_(x, y) x##y
#define CONCAT(x, y) CONCAT_(x, y)

#define DEF_INLINE_VAR_SINGLE(name) \
    template<typename T>            \
    inline constexpr bool CONCAT(name, _v) = name<T>::value

#define DEFAULT_MOVE(T) \
    T(T&&) = default;   \
    T& operator=(T&&) = default;

#define DELETE_COPY(T)    \
    T(const T&) = delete; \
    T& operator=(const T&) = delete;
