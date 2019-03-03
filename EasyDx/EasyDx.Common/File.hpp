#pragma once

#include "Win32Handles.hpp"
#include "FlagEnums.hpp"

namespace dx
{
    enum class FileShareMode
    {
        //Prevents other processes from opening a file or device if they request delete, read, or write access. 
        None = 0,
        Delete = 0x00000004,
        Read = 0x00000001,
        Write = 0x00000002
    };

    ENABLE_FLAGS(FileShareMode);

    enum class FileOpenMode : std::uint32_t
    {
        CreateAlways = 2,
        CreateNew = 1,
        OpenAlways = 4,
        OpenExisting = 3,
        TruncateExisting = 5
    };

    enum class FileAccessMode : std::uint32_t
    {
        Read = 0x80000000,
        Write = 0x40000000,
        Execute = 0x20000000,
        All = 0x10000000
    };

    FileHandle CreateOrOpenFile(const fs::path& path, FileAccessMode accessMode, FileOpenMode openMode, FileShareMode shareMode);

    inline FileHandle OpenExistingFile(const fs::path& path, FileAccessMode accessMode, FileShareMode shareMode)
    {
        return CreateOrOpenFile(path, accessMode, FileOpenMode::OpenExisting, shareMode);
    }
}