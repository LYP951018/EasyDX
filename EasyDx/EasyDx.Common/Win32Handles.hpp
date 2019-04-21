#pragma once

#include "UniqueHandle.hpp"
#include "Win32Def.hpp"

namespace dx
{
    struct FileHandleCloser
    {
        using HandleType = NativeHandle;
        static const NativeHandle InvalidHandle;

        void operator()(HandleType handle) noexcept;
    };

    using FileHandle = UniqueHandle<FileHandleCloser>;

    struct LocalMemoryHandleCloser
    {
        using HandleType = NativeHandle;
        static constexpr NativeHandle InvalidHandle = {};

        void operator()(HandleType handle) noexcept;
    };

    using LocalMemoryHandle = UniqueHandle<LocalMemoryHandleCloser>;

    // closing handles with ::CloseHandle
    struct CommonHandleCloser
    {
        using HandleType = NativeHandle;
        static constexpr NativeHandle InvalidHandle = {};

        void operator()(HandleType handle) noexcept;
    };

    using MemoryMappedFileHandle = UniqueHandle<CommonHandleCloser>;

    struct FileMappingViewUnmapper
    {
        using HandleType = void*;
        static constexpr NativeHandle InvalidHandle = {};

        void operator()(HandleType handle) noexcept;
    };

    using FileMappingViewHandle = UniqueHandle<FileMappingViewUnmapper>;

    template<typename UniqueHandleType>
    UniqueHandleType
    OpenWin32WithCheck(typename UniqueHandleType::HandleType handle)
    {
        auto uniqueHandle = UniqueHandleType{handle};
        if (!uniqueHandle)
        {
            ThrowWin32();
        }
        return std::move(uniqueHandle);
    }
} // namespace dx