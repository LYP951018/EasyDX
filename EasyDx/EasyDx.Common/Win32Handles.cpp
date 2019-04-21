#include "Win32Handles.hpp"

namespace dx
{
    void CommonHandleCloser::operator()(HandleType handle) noexcept
    {
        ::CloseHandle(handle);
    }

    void FileMappingViewUnmapper::operator()(HandleType memoryAddr) noexcept
    {
        [[maybe_unused]] const auto err = ::UnmapViewOfFile(memoryAddr);
        Ensures(err != 0);
    }

    void FileHandleCloser::operator()(HandleType handle) noexcept
    {
        ::CloseHandle(handle);
    }
} // namespace dx
