#pragma once

#include "Common.hpp"
#include <utility>
#include <cassert>

namespace dx
{
    template<typename Closer>
    class UniqueHandle
    {
      public:
        using HandleType = typename Closer::HandleType;

        UniqueHandle() noexcept : handle_{InvalidHandle()} {}

        explicit UniqueHandle(HandleType handle) noexcept : handle_{handle} {}

        DELETE_COPY(UniqueHandle)

        UniqueHandle(UniqueHandle&& other) noexcept : handle_{other.handle_}
        {
            other.handle_ = InvalidHandle();
        }

        UniqueHandle& operator=(UniqueHandle&& other) noexcept
        {
            UniqueHandle{std::move(other)}.swap(*this);
            return *this;
        }

        void swap(UniqueHandle& other) noexcept
        {
            using std::swap;
            swap(handle_, other.handle_);
        }

        constexpr explicit operator bool() const noexcept
        {
            return handle_ != InvalidHandle();
        }

        HandleType Get() const noexcept { return handle_; }

        HandleType* AddressOf() noexcept
        {
            assert(handle_ == InvalidHandle());
            return &handle_;
        }

        void Reset(HandleType other) noexcept
        {
            const auto old = Get();
            handle_ = other;
            if (old != InvalidHandle())
                Closer()(old);
        }

        HandleType Release() noexcept
        {
            const auto oldHandle = Get();
            handle_ = InvalidHandle();
            return oldHandle;
        }

        ~UniqueHandle()
        {
            if (handle_ != InvalidHandle())
                Closer()(handle_);
        }

        static /*constexpr */ HandleType InvalidHandle() noexcept
        {
            return Closer::InvalidHandle;
        }

      private:
        HandleType handle_;
    };

    template<typename T>
    void swap(UniqueHandle<T>& lhs, UniqueHandle<T>& rhs) noexcept
    {
        lhs.swap(rhs);
    }
} // namespace dx