#include "File.hpp"

namespace dx
{
    const NativeHandle FileHandleCloser::InvalidHandle = INVALID_HANDLE_VALUE;

    FileHandle CreateOrOpenFile(const fs::path& path, FileAccessMode accessMode,
                                FileOpenMode openMode, FileShareMode shareMode)
    {
        auto fileHandle = FileHandle{::CreateFile(
            path.c_str(), static_cast<DWORD>(accessMode), static_cast<DWORD>(shareMode), nullptr,
            static_cast<DWORD>(openMode), FILE_ATTRIBUTE_NORMAL, nullptr)};
        if (!fileHandle)
        {
            ThrowWin32();
        }
        return std::move(fileHandle);
    }
} // namespace dx
