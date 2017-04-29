#pragma once

#include <cstddef>

namespace stlext
{
    void* AlignedAlloc(std::size_t size, std::size_t align);
    void AlignedFree(void* ptr) noexcept;

    template<typename T>
    class AlignAllocator
    {
    public:
        using value_type = T;
        using pointer = value_type*;

        pointer allocate(std::size_t n)
        {
            return static_cast<pointer>(AlignedAlloc(n * sizeof(value_type), alignof(value_type)));
        }

        void deallocate(pointer ptr, std::size_t n) noexcept
        {
            AlignedFree(ptr);
        }

        bool operator== (const AlignAllocator& rhs) noexcept { return true; }
        bool operator!= (const AlignAllocator& rhs) noexcept { return !(*this == rhs); }
    };
}