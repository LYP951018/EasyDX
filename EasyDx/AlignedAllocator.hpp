#pragma once

namespace dx
{
    void* AlignedAlloc(std::size_t size, std::size_t align);
    void AlignedFree(void* ptr) noexcept;

    template<typename T>
    class AlignAllocator
    {
    public:
        using value_type = T;
        using pointer = value_type * ;

        AlignAllocator() = default;

        template<typename U>
        AlignAllocator(const AlignAllocator<U>&) {}

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

    template<typename T>
    using AlignedVec = std::vector<T, AlignAllocator<T>>;


    template<typename T>
    using aligned_unique_ptr = std::unique_ptr<T, void(&)(void*)>;

    template<typename T>
    aligned_unique_ptr<T> aligned_unique()
    {
        return { static_cast<std::add_pointer_t<T>>(dx::AlignedAlloc(sizeof(T), alignof(T))), dx::AlignedFree };
    }

    template<typename T>
    aligned_unique_ptr<T[]> aligned_unique(std::size_t n)
    {
        return { static_cast<std::add_pointer_t<T>>(dx::AlignedAlloc(sizeof(T) * n, alignof(T))), dx::AlignedFree };
    }

}