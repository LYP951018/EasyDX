//#pragma once
//
//#include <array>
//#include <cassert>
//#include "Common.hpp"
//
// namespace dx
//{
//    template<typename T, std::size_t N>
//    class static_vector
//    {
//        using StorageType = std::array<std::aligned_storage_t<sizeof(T), alignof(T)>, N>;
//        using TypedStorage = std::array<T, N>;
//
//    public:
//        using value_type = T;
//        using iterator = typename TypedStorage::iterator;
//        using const_iterator = typename TypedStorage::const_iterator;
//
//        static_vector()
//            : current_{storage_.begin()}
//        {}
//
//        void push_back(const T& v)
//        {
//            construct_at(pos(), v);
//            ++current_;
//        }
//
//        void push_back(T&& v)
//        {
//            construct_at(pos(), std::move(v));
//            ++current_;
//        }
//
//        T& operator[](std::size_t n) noexcept
//        {
//            assert(n < size());
//            return storage_[n];
//        }
//
//        const T& operator[](std::size_t n) const noexcept
//        {
//            assert(n < size());
//            return storage_[n];
//        }
//
//        T* data() noexcept
//        {
//            return reinterpret_cast<T*>(storage_.data());
//        }
//
//        const T* data() const noexcept
//        {
//            return reinterpret_cast<const T*>(storage_.data());
//        }
//
//        std::size_t size() const noexcept
//        {
//            return static_cast<std::size_t>(end() - begin());
//        }
//
//        constexpr std::size_t capacity() const noexcept
//        {
//            return N;
//        }
//
//        iterator begin() noexcept
//        {
//            return  storage_.begin();
//        }
//
//        const_iterator begin() const noexcept
//        {
//            return storage_.begin();
//        }
//
//        iterator end() noexcept
//        {
//            return current_;
//        }
//
//        const_iterator end() const noexcept
//        {
//            return current_;
//        }
//
//    private:
//        T * pos() const noexcept { return &*current_; }
//
//        StorageType storage_;
//        iterator current_;
//    };
//}