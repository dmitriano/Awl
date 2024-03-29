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

        PosixStream(FileHandle&& h) : m_hFile(std::forward<FileHandle>(h))
        {
        }

        bool operator == (const PosixStream& other) const
        {
            return m_hFile == other.m_hFile;
        }

        size_t GetLength() const override
        {
            struct stat sb;

            Check(::fstat(m_hFile, &sb));
            
            return static_cast<size_t>(sb.st_size);
        }

        size_t GetPosition() const override
        {
            const off_t pos = ::lseek(m_hFile, 0, SEEK_CUR);

            Check(pos);
            
            return static_cast<size_t>(pos);
        }

        size_t Read(uint8_t* buffer, size_t count) override
        {
            const ssize_t read_count = ::read(m_hFile, buffer, count);

            Check(read_count);
            
            return static_cast<size_t>(read_count);
        }

        void Write(const uint8_t* buffer, size_t count) override
        {
            const ssize_t written_count = ::write(m_hFile, buffer, count);

            if (written_count == static_cast<ssize_t>(-1))
            {
                throw PosixException("::write failed. This may indicate that the disk is full.");
            }

            if (static_cast<size_t>(written_count) != count)
            {
                throw PosixException(format() << _T("Requested ") << count
                    << _T(" bytes, but actually written ") << written_count << _T("."));
            }
        }

        bool End() override
        {
            return GetPosition() == GetLength();
        }

        void Seek(std::size_t pos, bool begin = true) override
        {
            Check(::lseek(m_hFile, static_cast<off_t>(pos), begin ? SEEK_SET : SEEK_END));
        }

        void Move(std::ptrdiff_t offset) override
        {
            Check(::lseek(m_hFile, static_cast<off_t>(offset), SEEK_CUR));
        }

        void Flush() override
        {
            Check(::fsync(m_hFile));
        }

        void Truncate() override
        {
            const off_t pos = ::lseek(m_hFile, 0, SEEK_CUR);

            Check(pos);

            Check(::ftruncate(m_hFile, pos));
        }

        String GetFileName() const
        {
            /*
            char buf[PATH_MAX];
            if (fcntl(m_hFile, F_GETPATH, buf) != -1)
            {
                return buf;
            }
            */

            return _T("<unknown path>");
        }

    private:

        template <class T>
        static void Check(T val)
        {
            if (val == static_cast<T>(-1))
            {
                throw PosixException();
            }
        }
        
        FileHandle m_hFile;
    };

    using UniqueStream = PosixStream<UniqueFileHandle>;
    
    // using SharedStream = PosixStream<SharedFileHandle>;

    inline thread_local bool openedExisting;

    inline UniqueFileHandle CreateUniqueFile(const String& file_name)
    {
        // I did not find a better way in POSIX.
        openedExisting = access(file_name.c_str(), F_OK) != -1;
        
        // user readable and writable
        HANDLE hFile = ::open(file_name.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);

        if (hFile == NullHandleValue)
        {
            throw PosixException(format() << _T("Cannot open file ')" << file_name << "' for updating."));
        }

        return hFile;
    }

    inline bool OpenedExisting()
    {
        return openedExisting;
    }

    inline UniqueFileHandle OpenUniqueFile(const String& file_name)
    {
        HANDLE hFile = ::open(file_name.c_str(), O_RDONLY);

        if (hFile == NullHandleValue)
        {
            throw PosixException(format() << _T("Cannot open file ')" << file_name << "' for reading."));
        }

        return hFile;
    }
}
