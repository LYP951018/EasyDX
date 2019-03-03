#include "AlignedAllocator.hpp"
#include "pch.hpp"
#include <stdexcept>
#ifdef _MSC_VER
#include <malloc.h>
#endif

namespace dx
{
    void* AlignedAlloc(std::size_t size, std::size_t align)
    {
#ifdef _MSC_VER
        const auto ptr = _aligned_malloc(size, align);
        dx::ThrowIf<std::bad_alloc>(ptr == nullptr);
        return static_cast<unsigned char*>(ptr);
#endif
    }

    void AlignedFree(void* ptr) noexcept { _aligned_free(ptr); }
} // namespace dx
