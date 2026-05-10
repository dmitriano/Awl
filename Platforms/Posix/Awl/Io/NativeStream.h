/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Io/Handle.h"
#include "Awl/StringFormat.h"
#include "Awl/Io/Stream.h"
#include "Awl/Io/IoException.h"
#include "Awl/Io/NativeException.h"
// #include <linux/limits.h>
// #include <fcntl.h>

namespace awl::io
{
    template <class FileHandle>
    class PosixStream : public IoStream
    {
    public:

        PosixStream() = default;

        PosixStream(FileHandle&& h) : _hFile(std::forward<FileHandle>(h))
        {}

        bool operator == (const PosixStream& other) const
        {
            return _hFile == other._hFile;
        }

        size_t length() const override
        {
            struct stat sb;

            check(::fstat(_hFile, &sb));
            
            return static_cast<size_t>(sb.st_size);
        }

        size_t position() const override
        {
            const off_t pos = ::lseek(_hFile, 0, SEEK_CUR);

            check(pos);
            
            return static_cast<size_t>(pos);
        }

        size_t read(uint8_t* buffer, size_t count) override
        {
            const ssize_t read_count = ::read(_hFile, buffer, count);

            check(read_count);
            
            return static_cast<size_t>(read_count);
        }

        void write(const uint8_t* buffer, size_t count) override
        {
            const ssize_t written_count = ::write(_hFile, buffer, count);

            if (written_count == static_cast<ssize_t>(-1))
            {
                throw PosixException("::write failed. This may indicate that the disk is full.");
            }

            if (static_cast<size_t>(written_count) != count)
            {
                throw PosixException(std::format(_T("Requested {} bytes, but actually written {}."), count, written_count));
            }
        }

        bool end() override
        {
            return position() == length();
        }

        void seek(std::size_t pos, bool begin = true) override
        {
            check(::lseek(_hFile, static_cast<off_t>(pos), begin ? SEEK_SET : SEEK_END));
        }

        void move(std::ptrdiff_t offset) override
        {
            check(::lseek(_hFile, static_cast<off_t>(offset), SEEK_CUR));
        }

        void flush() override
        {
            check(::fsync(_hFile));
        }

        void truncate() override
        {
            const off_t pos = ::lseek(_hFile, 0, SEEK_CUR);

            check(pos);

            check(::ftruncate(_hFile, pos));
        }

        String fileName() const
        {
            /*
            char buf[PATH_MAX];
            if (fcntl(_hFile, F_GETPATH, buf) != -1)
            {
                return buf;
            }
            */

            return _T("<unknown path>");
        }

    private:

        template <class T>
        static void check(T val)
        {
            if (val == static_cast<T>(-1))
            {
                throw PosixException();
            }
        }
        
        FileHandle _hFile;
    };

    using UniqueStream = PosixStream<UniqueFileHandle>;
    
    using SharedStream = PosixStream<SharedFileHandle>;

    inline thread_local bool openedExistingFlag;

    inline UniqueFileHandle createUniqueFile(const String& file_name)
    {
        // I did not find a better way in POSIX.
        openedExistingFlag = access(file_name.c_str(), F_OK) != -1;
        
        // user readable and writable
        HANDLE hFile = ::open(file_name.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);

        if (hFile == NullHandleValue)
        {
            throw PosixException(std::format(_T("Cannot open file '{}' for updating."), file_name));
        }

        return hFile;
    }

    inline bool openedExisting()
    {
        return openedExistingFlag;
    }

    inline UniqueFileHandle openUniqueFile(const String& file_name)
    {
        HANDLE hFile = ::open(file_name.c_str(), O_RDONLY);

        if (hFile == NullHandleValue)
        {
            throw PosixException(std::format(_T("Cannot open file '{}' for reading."), file_name));
        }

        return hFile;
    }
}
