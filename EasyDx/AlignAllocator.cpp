#include "AlignAllocator.h"
#include "Common.hpp"
#include <stdexcept>
#include <malloc.h>

namespace stlext
{
    void* AlignedAlloc(std::size_t size, std::size_t align)
    {
#ifdef _MSC_VER 
        const auto ptr = _aligned_malloc(size, align);
        dx::ThrowIf<std::bad_alloc>(ptr == nullptr);
        return static_cast<unsigned char*>(ptr);
#endif
    }

    void AlignedFree(void* ptr) noexcept
    {
        _aligned_free(ptr);
    }
}

